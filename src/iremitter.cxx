#include "iremitter.hxx"
#include "ast.hxx"

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include <iostream>
#include <list>
#include <system_error>

/* DEBUG */
#define __dump(_v_) (_v_)->print(llvm::errs(), false)

namespace basic {
///
IrEmitter::IrEmitter()
{
    // նախապատրաստել գրադարանային (սպասարկող) ֆունկցիաները
    prepareLibrary();
}

///
bool IrEmitter::emit(ProgramPtr prog, const std::filesystem::path& onm)
{
    try {
        emit(prog);

        std::error_code ec;
        llvm::raw_fd_ostream _fout(onm.string(), ec, llvm::sys::fs::OF_None);

        llvm::legacy::PassManager passer;
        passer.add(llvm::createPrintModulePass(_fout, ""));
        passer.run(*module);
        
        ///* DEBUG */ module->print(llvm::errs(), nullptr);
    }
    catch(...) {
        return false;
    }
    
    return true;
}

///
void IrEmitter::emit(ProgramPtr prog)
{
    // ստեղծել LLVM-ի Module օբյեկտ՝ դրա հասցեն պահելով
    // STL գրադարանի unique_ptr-ի մեջ։
    module = std::make_unique<llvm::Module>(prog->filename, context);

    // սեփական ֆունկցիաների հայտարարությունն ու սահմանումը
    // հարմար է առանձնացնել, որպեսզի Apply և Call գործողությունների
    // գեներացիայի ժամանակ արդեն գոյություն ունենան LLVM-ի
    // Function օբյեկտները
    
    // հայտարարել սեփական ֆունկցիաները
    declareSubroutines(prog);
    //  սահմանել սեփական ֆունկցիաները
    defineSubroutines(prog);

    // ավելացնել մուտքի կետը՝ main()
    createEntryPoint();
    
    // աշխատեցնել verify pass մոդուլի համար
    llvm::verifyModule(*module);
}

//
void IrEmitter::emit(SubroutinePtr subr)
{
    // մոդուլից վերցնել ֆունկցիայի հայտարարությունը դրան
    // մարմին ավելացնելու համար
    auto* func = module->getFunction(subr->name);

    // Քանի որ նախ գեներացվելու են ֆունկցիաների 
    // հայտարարությունները, ապա նույն այդ ցուցակով 
    // ամեն մի ֆունկցիայի համար գեներացվելու է մարմին,
    // բացառված է, որ fun ցուցիչը զրոյական լինի, սակայն,
    // կոդի ճիշտ կազմակերպվածության տեսակետից, ճիշտ կլինի,
    // որ այս և սրա նման դեպքերում աշխատանքը շարունակվի
    // ցուցիչի ոչ զրոյական լինելը ստուգելուց հետո
    if( nullptr == func )
        return;

    // ֆունկցիայի առաջին պիտակը (ցույց է տալիս ֆունկցիայի սկիզբը)
    auto* start = llvm::BasicBlock::Create(context, "start", func);
    builder.SetInsertPoint(start);

    // ֆունկցիայի պարամետրերին տալ սահմանված անունները
    for( auto& arg : func->args() ) {
        int ix = arg.getArgNo();
        arg.setName(subr->parameters[ix]);
    }

    // մաքրել varAddresses ցուցակը
    varAddresses.clear();

    // տեքստային օբյեկտների հասցեները
    std::list<llvm::Value*> localTexts;
    
    // բոլոր լոկալ փոփոխականների, պարամետրերի 
    // և վերադարձվող արժեքի համար
    for( const auto& vi : subr->locals ) {
        auto* vty = vi->type == Type::Boolean 
                              ? builder.getInt8Ty()
                              : llvmType(vi->type); // REVIEW
        auto* addr = builder.CreateAlloca(vty, nullptr, vi->name + "_addr");
        varAddresses[vi->name] = addr;
        if( vi->is(Type::Textual) )
            localTexts.push_back(addr);
    }

    // պարամետրերի արժեքները վերագրել լոկալ օբյեկտներին
    for( auto& arg : func->args() )
        if( arg.getType()->isPointerTy() ) {
            auto parval = createLibraryFuncCall("text_clone", { &arg });
            builder.CreateStore(parval, varAddresses[arg.getName().str()]);
            localTexts.remove(varAddresses[arg.getName().str()]);
        }
        else
            builder.CreateStore(&arg, varAddresses[arg.getName().str()]);

    // տեքստային օբյեկտների համար գեներացնել սկզբնական արժեք
    // (սա արվում է վերագրման ժամանակ հին արժեքը ջնջելու և 
    // նորը վերագրելու սիմետրիկությունն ապահովելու համար)
    auto one = builder.getInt64(1);
    for( auto* vp : localTexts ) {
        auto deva = createLibraryFuncCall("malloc", { one });
        builder.CreateStore(deva, vp);
    }

    // գեներացնել ենթածրագրի մարմնի հրամանները
    emit(subr->body);

    // ազատել տեքստային օբյեկտների զբաղեցրած հիշողությունը
    // Յուրաքանչյուր ֆունկցիայի ավարտին պետք է ազատել
    // տեքստային օբյեկտների զբաղեցրած հիշողությունը։ 
    // Բացառություն պիտի լինի միայն ֆունկցիայի անունով 
    // փոփոխականին կապված արժեքը, որը վերադարձվելու է
    // ֆունկցիային կանչողին
    for( auto& vi : subr->locals ) {
        if( vi->name == subr->name )
            continue;

        if( vi->is(Type::Numeric) || vi->is(Type::Boolean) )
            continue;

		if( Type::Textual == vi->type ) {
		    auto addr = builder.CreateLoad(TextualTy, varAddresses[vi->name]);
		    createLibraryFuncCall("free", { addr });
		}
    }

    // վերադարձվող արժեք
    if( func->getReturnType()->isVoidTy() )
        builder.CreateRetVoid();
    else {
        auto rv = builder.CreateLoad(func->getReturnType(), varAddresses[subr->name]);
        builder.CreateRet(rv);
    }

    // ստուգել կառուցված ֆունկցիան
    llvm::verifyFunction(*func);
}

///
void IrEmitter::emit(StatementPtr st)
{
    switch( st->kind ) {
        case NodeKind::Sequence:
            emit(std::dynamic_pointer_cast<Sequence>(st));
            break;
        case NodeKind::Input:
            emit(std::dynamic_pointer_cast<Input>(st));
            break;
        case NodeKind::Print:
            emit(std::dynamic_pointer_cast<Print>(st));
            break;
        case NodeKind::Let:
            emit(std::dynamic_pointer_cast<Let>(st));
            break;
        case NodeKind::If:
            emit(std::dynamic_pointer_cast<If>(st));
            break;
        case NodeKind::While:
            emit(std::dynamic_pointer_cast<While>(st));
            break;
        case NodeKind::For:
            emit(std::dynamic_pointer_cast<For>(st));
            break;
        case NodeKind::Call:
            emit(std::dynamic_pointer_cast<Call>(st));
            break;
        default:
            break;
    }
}

///
void IrEmitter::emit(SequencePtr seq)
{
    for( auto& st : seq->items )
        emit(st);
}

///
void IrEmitter::emit(LetPtr let)
{
    auto* val = emit(let->expr);
    auto* addr = varAddresses[let->place->name];
    
    if( let->place->is(Type::Textual) ) {
        auto* dera = builder.CreateLoad(TextualTy, addr);  
        createLibraryFuncCall("free", {dera});
        if( !createsTempText(let->expr) )
            val = createLibraryFuncCall("text_clone", { val });
    }
    else if( let->place->is(Type::Boolean) ) {
        val = builder.CreateZExt(val, builder.getInt8Ty());
    }

    builder.CreateStore(val, addr);
}

///
void IrEmitter::emit(InputPtr inp)
{
    // ստանալ հրավերքի տեքստի հասցեն
    auto* prompt = emit(inp->prompt);

    // հաշվարկել ներմուծող ֆունկցիան
    std::string_view funcName;
    if( inp->place->is(Type::Boolean) )
        funcName = "bool_input";
    if( inp->place->is(Type::Numeric) )
        funcName = "number_input";
    else if( inp->place->is(Type::Textual) )
        funcName = "text_input";

    // գեներացնել ներմուծող ֆունկցիայի կանչ
    auto* val = createLibraryFuncCall(funcName, {prompt});
    // ներմուծված արժեքը վերագրել համապատասխան հասցեին
    builder.CreateStore(val, varAddresses[inp->place->name]);
}

///
void IrEmitter::emit(PrintPtr pri)
{
    // արտածվող արտահայտության կոդը
    auto* expr = emit(pri->expr);
    
    if( pri->expr->is(Type::Boolean) ) {
        //createLibraryFuncCall("bool_print", {expr});
    }
    else if( pri->expr->is(Type::Textual) ) {
        createLibraryFuncCall("text_print", {expr});
        if( createsTempText(pri->expr) )
            createLibraryFuncCall("free", {expr});
    }
    else if( pri->expr->is(Type::Numeric) )
        createLibraryFuncCall("number_print", {expr});
}

///
void IrEmitter::emit(IfPtr sif)
{
    // ընթացիկ ֆունկցիայի ցուցիչը
    auto* func = builder.GetInsertBlock()->getParent();

    // ճյուղավորման ամենավերջին բլոկը
    auto* endIf = llvm::BasicBlock::Create(context, "", func);
    
    // if֊երի շղթայի առաջին բլոկը
    auto* first = llvm::BasicBlock::Create(context, "", func, endIf);
    setCurrentBlock(func, first);
    
    StatementPtr sp = sif;
    while( auto ifp = std::dynamic_pointer_cast<If>(sp) ) {
        // then֊բլոկ
        auto* thenBlock = llvm::BasicBlock::Create(context, "", func, endIf);
        // else-բլոկ
        auto* elseBlock = llvm::BasicBlock::Create(context, "", func, endIf);

        // գեներացնել պայմանը 
        auto* cnd = emit(ifp->condition);
		//cnd = builder.CreateFCmpUNE(cnd, Zero);
        
        // անցում ըստ պայմանի
        builder.CreateCondBr(cnd, thenBlock, elseBlock);

        // then-ի հրամաններ
        setCurrentBlock(func, thenBlock);

        emit(ifp->decision);
        builder.CreateBr(endIf);

        // պատրաստվել հաջորդ բլոկին
        setCurrentBlock(func, elseBlock);
        
        // հաջորդ բլոկի մշակում
        sp = ifp->alternative;
    }
    
    // կա՞ արդյոք else-բլոկ
    if( nullptr != sp )
        emit(sp);
    
    setCurrentBlock(func, endIf);
}

///
void IrEmitter::emit(WhilePtr swhi)
{
    // ընթացիկ ֆունկցիան
    auto* func = builder.GetInsertBlock()->getParent();

    // ցիկլի պայմանի, մարմնի և ավարտի բլոկները
    auto* condBlock = llvm::BasicBlock::Create(context, "", func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, "", func);
    auto* endWhile = llvm::BasicBlock::Create(context, "", func);

    setCurrentBlock(func, condBlock);

    // գեներացնել կրկնման պայմանը
    auto* condEx = emit(swhi->condition);
    builder.CreateCondBr(condEx, bodyBlock, endWhile);

    setCurrentBlock(func, bodyBlock);

    // գեներացնել ցիկլի մարմինը
    emit(swhi->body);
    builder.CreateBr(condBlock);

    setCurrentBlock(func, endWhile);
}

///
void IrEmitter::emit(ForPtr sfor)
{
    // ընթացիկ ֆունկցիան
    auto* func = builder.GetInsertBlock()->getParent();

    auto* condBlock = llvm::BasicBlock::Create(context, "", func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, "", func);
    auto* endFor = llvm::BasicBlock::Create(context, "", func);

    // ցիկլի պարամետրի հասցեն
    auto* param = varAddresses[sfor->parameter->name];
    // գեներացնել սկզբնական արժեքի արտահայտությունը
    auto* init = emit(sfor->begin);
    // պարամետրին վերագրել սկզբնական արժեքը
    builder.CreateStore(init, param);
    // գեներացնել վերջնական արժեքի արտահայտությունը
    auto* finish = emit(sfor->end);
    // քայլը հաստատուն թիվ է
    auto* step = llvm::ConstantFP::get(NumericTy, sfor->step->value);
    
    setCurrentBlock(func, condBlock);

    // եթե պարամետրի արժեքը >= (կամ <=, եթե քայլը բացասական է)
    // վերջնականից, ապա ավարտել ցիկլը
    auto* parVal = builder.CreateLoad(NumericTy, param);
    llvm::Value* coex = nullptr;
    if( sfor->step->value > 0.0 ) {
        coex = builder.CreateFCmpOLT(parVal, finish);
    }
    else if( sfor->step->value < 0.0 ) {
        coex = builder.CreateFCmpOGT(parVal, finish);
    }
    builder.CreateCondBr(coex, bodyBlock, endFor);

    setCurrentBlock(func, bodyBlock);

    // գեներացնել մարմինը
    emit(sfor->body);

    // պարամետրի արժեքին գումարել քայլի արժեքը
    auto* parval = builder.CreateLoad(NumericTy, param);
    auto* nwpv = builder.CreateFAdd(parval, step);
    builder.CreateStore(nwpv, param);

    // կրկնել ցիկլը
    builder.CreateBr(condBlock);

    setCurrentBlock(func, endFor);
}

///
void IrEmitter::emit(CallPtr cal)
{
    // պրոցեդուրայի կանչը նույն ֆունկցիայի կիրառումն է
    emit(cal->subrCall);
}

///
llvm::Value* IrEmitter::emit(ExpressionPtr expr)
{
    llvm::Value* res = nullptr;

    switch( expr->kind ) {
        case NodeKind::Boolean:
            res = emit(std::dynamic_pointer_cast<Boolean>(expr));
            break;
        case NodeKind::Number:
            res = emit(std::dynamic_pointer_cast<Number>(expr));
            break;
        case NodeKind::Text:
            res = emit(std::dynamic_pointer_cast<Text>(expr));
            break;
        case NodeKind::Variable:
            res = emit(std::dynamic_pointer_cast<Variable>(expr));
            break;
        case NodeKind::Unary:
            res = emit(std::dynamic_pointer_cast<Unary>(expr));
            break;
        case NodeKind::Binary:
            res = emit(std::dynamic_pointer_cast<Binary>(expr));
            break;
        case NodeKind::Apply:
            res = emit(std::dynamic_pointer_cast<Apply>(expr));
            break;
        default:
            break;
    }

    return res;
}

///
llvm::Value* IrEmitter::emit(TextPtr txt)
{
    // եթե տրված արժեքով տող արդեն սահմանված է գլոբալ
    // տիրույթում, ապա վերադարձնել դրա հասցեն
    if( const auto sri = globalTexts.find(txt->value); sri != globalTexts.end() )
        return sri->second;

    // ... հակառակ դեպքում՝ սահմանել նոր գլոբալ տող, դրա հասցեն
    // պահել գլոբալ տողերի ցուցակում և վերադարձնել որպես արժեք
    auto* strp = builder.CreateGlobalStringPtr(txt->value, "g_str");
    globalTexts[txt->value] = strp;

    return strp;
}

///
llvm::Constant* IrEmitter::emit(NumberPtr num)
{
    // գեներացնել թվային հաստատուն
    return llvm::ConstantFP::get(builder.getDoubleTy(), num->value);
}

///
llvm::Constant* IrEmitter::emit(BooleanPtr bv)
{
    // գեներացնել տրամաբանական հաստատուն
    return llvm::ConstantInt::getBool(builder.getInt1Ty(), bv->value);
}

///
llvm::UnaryInstruction* IrEmitter::emit(VariablePtr var)
{
    // ստանալ փոփոխականի հասցեն ...
    auto* vaddr = varAddresses[var->name];

    // ... և գեներացնել արժեքի բեռնման հրահանգ
    if( var->is(Type::Boolean) ) {
        llvm::Type* ByteType = builder.getInt8Ty();
        llvm::LoadInst* res = builder.CreateLoad(ByteType, vaddr, var->name);
        return llvm::dyn_cast<llvm::UnaryInstruction>(builder.CreateTrunc(res, BooleanTy)); // REVIEW
    }

    return builder.CreateLoad(llvmType(var->type), vaddr, var->name);; 
}

///
llvm::Value* IrEmitter::emit(ApplyPtr apy)
{
    // գեներացնել կանչի արգումենտները
    llvm::SmallVector<llvm::Value*> argus, temps;
    for( const auto& ai : apy->arguments ) {
        auto ap = emit(ai);
        argus.push_back(ap);
        if( createsTempText(ai) )
            temps.push_back(ap);
    }

    // կանչել ֆունկցիան ու պահել արժեքը
    //auto callee = module->getFunction(apy->procptr->name);
	auto callee = userFunction(apy->callee->name);
    auto calv = builder.CreateCall(callee, argus);

    // մաքրել կանչի ժամանակավոր արգումենտները
    for( auto* ai : temps )
        if( ai->getType()->isPointerTy() )
            createLibraryFuncCall("free", { ai });

    // վերադարձնել կանչի արդյունքը
    return calv;
}

/*
Համեմատման ու տրամաբանական գործողությունների համար կոդ գեներացնելիս
ստացված արդյունքը ստիպված եմ եղել _բուլյան_ (`i1`) արժեքը ձևափոխել 
_իրական_ (`double`) արժեքի։ Սակայն, քանի որ `IF` հրամանների `WHILE`
պայմանները պետք է լինեն `i1` տիպի, նորից ստիպված եմ եղել իրական
արժեքը դարձնել բուլյան։ Սա կարելի է շտկել կամ կոդի գեներացիայի
ժամանակ, կամ էլ թողնել օպտիմիզատորին։ Առաջին տարբերակն, իհարկե,
նախընտրելի է։ 
*/
///
llvm::Value* IrEmitter::emit(BinaryPtr bin)
{
    const bool textuals = bin->left->is(Type::Textual)
                       && bin->right->is(Type::Textual);
    const bool numerics = bin->left->is(Type::Numeric)
                       && bin->right->is(Type::Numeric);
    const bool booleans = bin->left->is(Type::Boolean)
                       && bin->right->is(Type::Boolean);

    auto* lhs = emit(bin->left);
    auto* rhs = emit(bin->right);
	
    llvm::Value* ret = nullptr;
    switch( bin->opcode ) {
        case Operation::Add:
            ret = builder.CreateFAdd(lhs, rhs, "add");
            break;
        case Operation::Sub:
            ret = builder.CreateFSub(lhs, rhs, "sub");
            break;
        case Operation::Mul:
            ret = builder.CreateFMul(lhs, rhs, "mul");
            break;
        case Operation::Div:
            ret = builder.CreateFDiv(lhs, rhs, "div");
            break;
        case Operation::Mod:
            ret = builder.CreateFRem(lhs, rhs, "rem");
            break;
        case Operation::Pow:
            ret = createLibraryFuncCall("pow", {lhs, rhs});
            break;

        case Operation::Eq:
            if( textuals )
                ret = createLibraryFuncCall("text_eq", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOEQ(lhs, rhs, "eq");
            else if( booleans )
                ret = builder.CreateICmpEQ(lhs, rhs, "eq");
            break;
        case Operation::Ne:
            if( textuals )
                ret = createLibraryFuncCall("text_ne", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpONE(lhs, rhs, "ne");
            else if( booleans )
                ret = builder.CreateICmpNE(lhs, rhs, "ne");
            break;
        case Operation::Gt:
            if( textuals )
                ret = createLibraryFuncCall("text_gt", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOGT(lhs, rhs, "gt");
            break;
        case Operation::Ge:
            if( textuals )
                ret = createLibraryFuncCall("text_ge", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOGE(lhs, rhs, "ge");
            break;
        case Operation::Lt:
            if( textuals )
                ret = createLibraryFuncCall("text_lt", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOLT(lhs, rhs, "lt");
            break;
        case Operation::Le:
            if( textuals )
                ret = createLibraryFuncCall("text_le", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOLE(lhs, rhs, "le");
            break;

        case Operation::And:
            ret = builder.CreateAnd(lhs, rhs, "and");
            break;
        case Operation::Or:
            ret = builder.CreateOr(lhs, rhs, "or");
            break;

        case Operation::Conc:
            ret = createLibraryFuncCall("text_conc", {lhs, rhs});
            break;
        default:
            break;
    }
		
    return ret;
}

///
llvm::Value* IrEmitter::emit(UnaryPtr un)
{
    // գեներացնել ենթաարտահայտությունը
    auto* val = emit(un->subexpr);

    // ունար մինուս (բացասում)
    if( Operation::Sub == un->opcode )
        return builder.CreateFNeg(val, "neg");

    // ժխտում
    if( Operation::Not == un->opcode )
	  return builder.CreateNot(val);
    
    return val;
}

///
void IrEmitter::setCurrentBlock(llvm::Function* fun, llvm::BasicBlock* bl)
{
    if( auto* ib = builder.GetInsertBlock(); nullptr != ib && nullptr == ib->getTerminator() )
        builder.CreateBr(bl);
    
    builder.ClearInsertionPoint();

    // auto _ib = builder.GetInsertBlock();
    // if( nullptr != _ib && nullptr != _ib->getParent() )
    //     fun->getBasicBlockList().insertAfter(_ib->getIterator(), bl);
    // else
    //     fun->getBasicBlockList().push_back(bl);
    
    fun->getBasicBlockList().push_back(bl);
    
    builder.SetInsertPoint(bl);
}

///
void IrEmitter::prepareLibrary()
{
    auto* _V = builder.getVoidTy();
    auto* _B = builder.getInt1Ty();
    auto* _N = builder.getDoubleTy();
    auto* _T = builder.getInt8PtrTy();

    // տեքստային ֆունկցիաներ
    library["text_clone"] = llvm::FunctionType::get(_T, {_T}, false);
    library["text_input"] = llvm::FunctionType::get(_T, {_T}, false);
    library["text_print"] = llvm::FunctionType::get(_V, {_T}, false);
    library["text_conc"] = llvm::FunctionType::get(_T, {_T, _T}, false);
    library["text_mid"] = llvm::FunctionType::get(_T, {_T, _N, _N}, false);
    library["text_str"] = llvm::FunctionType::get(_T, {_N}, false);
    library["text_eq"] = llvm::FunctionType::get(_B, {_T, _T}, false);
    library["text_ne"] = llvm::FunctionType::get(_B, {_T, _T}, false);
    library["text_gt"] = llvm::FunctionType::get(_B, {_T, _T}, false);
    library["text_ge"] = llvm::FunctionType::get(_B, {_T, _T}, false);
    library["text_lt"] = llvm::FunctionType::get(_B, {_T, _T}, false);
    library["text_le"] = llvm::FunctionType::get(_B, {_T, _T}, false);

    // թվային ֆունկցիաներ
    library["number_input"] = llvm::FunctionType::get(_N, {_T}, false);
    library["number_print"] = llvm::FunctionType::get(_V, {_N}, false);

    // մաթեմատիկական ֆունկցիաներ
    library["pow"] = llvm::FunctionType::get(_N, {_N, _N}, false);
    library["sqrt"] = llvm::FunctionType::get(_N, {_N}, false);

    // հիշողության ֆունկցիաներ
    library["malloc"] = llvm::FunctionType::get(_T, {builder.getInt64Ty()}, false);
    library["free"] = llvm::FunctionType::get(_V, {_T}, false);
}

///
llvm::FunctionCallee IrEmitter::libraryFunction(std::string_view name)
{
    return module->getOrInsertFunction(name, library[std::string{name}]);
}

///
llvm::FunctionCallee IrEmitter::userFunction(std::string_view name)
{
    if( "MID$" == name )
        return libraryFunction("text_mid");

    if( "STR$" == name )
        return libraryFunction("text_str");

    if( "SQR" == name )
        return libraryFunction("sqrt");
	
    return module->getFunction(name);
}

///
void IrEmitter::createEntryPoint()
{
    auto* Int32Ty = builder.getInt32Ty();

    auto mainType = llvm::FunctionType::get(Int32Ty, {}, false);
    const auto linkage = llvm::GlobalValue::ExternalLinkage;
    auto mainFunc = llvm::Function::Create(mainType, linkage, "main", module.get());

    auto* start  = llvm::BasicBlock::Create(context, "start", mainFunc);
    builder.SetInsertPoint(start);

    // եթե ծրագրավորողը սահմանել է Main անունով ենթածրագիր, ապա
    // main()-ի մեջ կանչել այն, հակառակ դեպքում main-ը դատարկ է
    if( auto* udMain = module->getFunction("Main"); nullptr != udMain )
        builder.CreateCall(udMain, {});
    
    auto* returnValue = llvm::ConstantInt::get(Int32Ty, 0);
    builder.CreateRet(returnValue);
}        

///
void IrEmitter::declareSubroutines(ProgramPtr prog)
{
    for( const auto& subr : prog->members ) {
        // պարամետրերի տիպերի ցուցակի կառուցումը
        llvm::SmallVector<llvm::Type*> paramTypes;
        for( const auto& pr : subr->parameters )
            paramTypes.push_back(llvmType(pr));

        // վերադարձվող արժեքի տիպը
        llvm::Type* returnType = subr->hasValue ? 
                llvmType(subr->name) :
                builder.getVoidTy();

        // ստեղծել ֆունկցիայի հայտարարությունը
        auto* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        const auto linkage = llvm::GlobalValue::ExternalLinkage;
        llvm::Function::Create(funcType, linkage, subr->name, module.get());
    }
}

///
void IrEmitter::defineSubroutines(ProgramPtr prog)
{
    for( const auto& subr : prog->members )
        if( !subr->isBuiltIn )
            emit(subr);
}

///
llvm::Type* IrEmitter::llvmType(Type type)
{
    if( Type::Boolean == type )
        return BooleanTy;

    if( Type::Numeric == type )
        return NumericTy;

    if( Type::Textual == type )
        return TextualTy;

    return VoidTy;
}

///
llvm::Type* IrEmitter::llvmType(std::string_view name)
{
    return llvmType(typeOf(name));
}

///
bool IrEmitter::createsTempText(ExpressionPtr expr)
{
    // թվային արտահայտությունը ժամանակավոր օբյեկտ չի ստեղծում
    if( expr->is(Type::Numeric) || expr->is(Type::Boolean) )
        return false;

    // տեքստային լիտերալներն ու փոփոխականներն էլ չեն ստեղծում
    if( NodeKind::Text == expr->kind || NodeKind::Variable == expr->kind )
        return false;

    return true;
}

///
llvm::CallInst* IrEmitter::createLibraryFuncCall(std::string_view fname, 
            const llvm::ArrayRef<llvm::Value*>& args)
{
    return builder.CreateCall(libraryFunction(fname), args);
}

} // namespace llvm
