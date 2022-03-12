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
#include <sstream>
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
bool IrEmitter::emitIr(ProgramPtr prog, const std::filesystem::path& onm)
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
    auto fun = module->getFunction(subr->name);

    // Քանի որ նախ գեներացվելու են ֆունկցիաների 
    // հայտարարությունները, ապա նույն այդ ցուցակով 
    // ամեն մի ֆունկցիայի համար գեներացվելու է մարմին,
    // բացառված է, որ fun ցուցիչը զրոյական լինի, սակայն,
    // կոդի ճիշտ կազմակերպվածության տեսակետից, ճիշտ կլինի,
    // որ այս և սրա նման դեպքերում աշխատանքը շարունակվի
    // ցուցիչի ոչ զրոյական լինելը ստուգելուց հետո
    if( nullptr == fun )
        return;

    // ֆունկցիայի առաջին պիտակը (ցույց է տալիս ֆունկցիայի սկիզբը)
    auto* start = llvm::BasicBlock::Create(context, "start", fun);
    builder.SetInsertPoint(start);

    // ֆունկցիայի պարամետրերին տալ սահմանված անունները
    for( auto& arg : fun->args() ) {
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
        auto* vty = vi->type == Type::Boolean ? builder.getInt8Ty() : llvmType(vi->type); // REVIEW
        auto* addr = builder.CreateAlloca(vty, nullptr, vi->name + "_addr");
        varAddresses[vi->name] = addr;
        if( vi->is(Type::Textual) )
            localTexts.push_back(addr);
    }

    // պարամետրերի արժեքները վերագրել լոկալ օբյեկտներին
    for( auto& arg : fun->args() )
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
		    auto addr = builder.CreateLoad(TextType, varAddresses[vi->name]);
		    createLibraryFuncCall("free", { addr });
		}
    }

    // վերադարձվող արժեք
    if( fun->getReturnType()->isVoidTy() )
        builder.CreateRetVoid();
    else {
        auto rv = builder.CreateLoad(fun->getReturnType(), varAddresses[subr->name]);
        builder.CreateRet(rv);
    }

    // ստուգել կառուցված ֆունկցիան
    llvm::verifyFunction(*fun);
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
    for( auto st : seq->items )
        emit(st);
}

///
void IrEmitter::emit(LetPtr let)
{
    auto* val = emit(let->expr);
    auto* addr = varAddresses[let->place->name];
    
    if( Type::Textual == let->place->type ) {
        auto _dera = builder.CreateLoad(TextType, addr);  
        createLibraryFuncCall("free", {_dera});
        if( !createsTempText(let->expr) )
            val = createLibraryFuncCall("text_clone", { val });
    }
    else if( Type::Boolean == let->place->type ) {
        val = builder.CreateZExt(val, builder.getInt8Ty());
    }

    builder.CreateStore(val, addr);
}

///
void IrEmitter::emit(InputPtr inp)
{
    // ստանալ հրավերքի տեքստի հասցեն
    auto _probj = std::make_shared<Text>(inp->prompt);
    auto _pref = emit(_probj);

    // հաշվարկել ներմուծող ֆունկցիան
    std::string_view funcName;
    if( Type::Textual == inp->varptr->type )
        funcName = "text_input";
    else if( Type::Numeric == inp->varptr->type )
        funcName = "number_input";    

    // գեներացնել ներմուծող ֆունկցիայի կանչ
    auto _inp = createLibraryFuncCall(funcName, {_pref});
    // ներմուծված արժեքը վերագրել համապատասխան հասցեին
    builder.CreateStore(_inp, varAddresses[inp->varptr->name]);
}

///
void IrEmitter::emit(PrintPtr pri)
{
    // արտածվող արտահայտության կոդը
    auto _expr = emit(pri->expr);
    
    if( Type::Textual == pri->expr->type ) {
        createLibraryFuncCall("text_print", {_expr});
        if( createsTempText(pri->expr) )
            createLibraryFuncCall("free", {_expr});
    }
    else if( Type::Numeric == pri->expr->type )
        createLibraryFuncCall("number_print", {_expr});
}

///
void IrEmitter::emit(IfPtr sif)
{
    // ընթացիկ ֆունկցիայի դուրս բերում
    auto _fun = builder.GetInsertBlock()->getParent();

    // ճյուղավորման ամենավերջին բլոկը
    auto _eif = llvm::BasicBlock::Create(context, "", _fun);
    
    auto _first = llvm::BasicBlock::Create(context, "", _fun, _eif);
    setCurrentBlock(_fun, _first);
    
    StatementPtr sp = sif;
    while( auto _if = std::dynamic_pointer_cast<If>(sp) ) {
        // then֊բլոկ
        auto _tbb = llvm::BasicBlock::Create(context, "", _fun, _eif);
        // else-բլոկ
        auto _cbb = llvm::BasicBlock::Create(context, "", _fun, _eif);

        // գեներացնել պայմանը 
        auto cnd = emit(_if->condition);
		cnd = builder.CreateFCmpUNE(cnd, Zero);
        
        // անցում ըստ պայմանի
        builder.CreateCondBr(cnd, _tbb, _cbb);

        // then-ի հրամաններ
        setCurrentBlock(_fun, _tbb);

        emit(_if->decision);
        builder.CreateBr(_eif);

        // պատրաստվել հաջորդ բլոկին
        setCurrentBlock(_fun, _cbb);
        
        // հաջորդ բլոկի մշակում
        sp = _if->alternative;
    }
    
    // կա՞ արդյոք else-բլոկ
    if( nullptr != sp )
        emit(sp);
    
    setCurrentBlock(_fun, _eif);
}

///
void IrEmitter::emit(WhilePtr swhi)
{
    // ընթացիկ ֆունկցիան
    auto _fun = builder.GetInsertBlock()->getParent();

    // ցիկլի պայմանի, մարմնի և ավարտի բլոկները
    auto _cond = llvm::BasicBlock::Create(context, "", _fun);
    auto _body = llvm::BasicBlock::Create(context, "", _fun);
    auto _end = llvm::BasicBlock::Create(context, "", _fun);

    setCurrentBlock(_fun, _cond);

    // գեներացնել կրկնման պայմանը
    auto coex = emit(swhi->condition);
	coex = builder.CreateFCmpUNE(coex, Zero);
    builder.CreateCondBr(coex, _body, _end);

    setCurrentBlock(_fun, _body);

    // գեներացնել ցիկլի մարմինը
    emit(swhi->body);
    builder.CreateBr(_cond);

    setCurrentBlock(_fun, _end);
}

///
void IrEmitter::emit(ForPtr sfor)
{
    // ընթացիկ ֆունկցիան
    auto _fun = builder.GetInsertBlock()->getParent();

    auto _cond = llvm::BasicBlock::Create(context, "", _fun);
    auto _body = llvm::BasicBlock::Create(context, "", _fun);
    auto _end = llvm::BasicBlock::Create(context, "", _fun);

    auto _param = varAddresses[sfor->parameter->name];
    // գեներացնել սկզբնական արժեքի արտահայտությունը
    auto _init = emit(sfor->begin);
    // պարամետրին վերագրել սկզբնական արժեքը
    builder.CreateStore(_init, _param);
    // գեներացնել վերջնական արժեքի արտահայտությունը
    auto _finish = emit(sfor->end);
    // քայլը հաստատուն է
    auto _step = llvm::ConstantFP::get(builder.getDoubleTy(), sfor->step->value);
    
    setCurrentBlock(_fun, _cond);

    // եթե պարամետրի արժեքը >= (կամ <=, եթե քայլը բացասական է)
    // վերջնականից, ապա ավարտել ցիկլը
    if( sfor->step->value >= 0.0 ) {
        auto _pv = builder.CreateLoad(NumberType, _param);
        auto coex = builder.CreateFCmpOLT(_pv, _finish);
        builder.CreateCondBr(coex, _body, _end);
    }
    else if( sfor->step->value <= 0.0 ) {
        auto _pv = builder.CreateLoad(NumberType, _param);
        auto coex = builder.CreateFCmpOGT(_pv, _finish);
        builder.CreateCondBr(coex, _body, _end);
    }

    setCurrentBlock(_fun, _body);

    // գեներացնել մարմինը
    emit(sfor->body);

    // պարամետրի արժեքին գումարել քայլի արժեքը
    auto parval = builder.CreateLoad(NumberType, _param);
    auto nwpv = builder.CreateFAdd(parval, _step);
    builder.CreateStore(nwpv, _param);

    // կրկնել ցիկլը
    builder.CreateBr(_cond);

    setCurrentBlock(_fun, _end);
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
    auto sri = globalTexts.find(txt->value);
    if( sri != globalTexts.end() )
        return sri->second;

    // ... հակառակ դեպքում՝ սահմանել նոր գլոբալ տող, դրա հասցեն
    // պահել գլոբալ տողերի ցուցակում և վերադարձնել որպես արժեք
    auto strp = builder.CreateGlobalStringPtr(txt->value, "g_str");
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
    if( var->type == Type::Boolean ) {
        llvm::Type* ByteType = builder.getInt8Ty();
        llvm::LoadInst* res = builder.CreateLoad(ByteType, vaddr, var->name);
        return llvm::dyn_cast<llvm::UnaryInstruction>(builder.CreateTrunc(res, BooleanType)); // REVIEW
    }

    return builder.CreateLoad(llvmType(var->type), vaddr, var->name);; 
}

///
llvm::Value* IrEmitter::emit(ApplyPtr apy)
{
    // գեներացնել կանչի արգումենտները
    std::vector<llvm::Value*> argus, temps;
    for( auto& ai : apy->arguments ) {
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
    for( auto ai : temps )
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
    llvm::Value* lhs = emit(bin->left);
    llvm::Value* rhs = emit(bin->right);

    bool numopers = (Type::Numeric == bin->left->type)
                 || (Type::Numeric == bin->right->type);

    if( Operation::And == bin->opcode || Operation::Or == bin->opcode ) {
		lhs = builder.CreateFCmpUNE(lhs, Zero);
		rhs = builder.CreateFCmpUNE(rhs, Zero);
	}
	
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
            if( numopers )
                ret = builder.CreateFCmpOEQ(lhs, rhs, "eq");
            else
                ret = createLibraryFuncCall("text_eq", {lhs, rhs});
            break;
        case Operation::Ne:
            if( numopers )
                ret = builder.CreateFCmpONE(lhs, rhs, "ne");
            else
                ret = createLibraryFuncCall("text_ne", {lhs, rhs});
            break;
        case Operation::Gt:
            if( numopers )
                ret = builder.CreateFCmpOGT(lhs, rhs, "gt");
            else
                ret = createLibraryFuncCall("text_gt", {lhs, rhs});
            break;
        case Operation::Ge:
            if( numopers )
                ret = builder.CreateFCmpOGE(lhs, rhs, "ge");
            else
                ret = createLibraryFuncCall("text_ge", {lhs, rhs});
            break;
        case Operation::Lt:
            if( numopers )
                ret = builder.CreateFCmpOLT(lhs, rhs, "lt");
            else
                ret = createLibraryFuncCall("text_lt", {lhs, rhs});
            break;
        case Operation::Le:
            if( numopers )
                ret = builder.CreateFCmpOLE(lhs, rhs, "le");
            else
                ret = createLibraryFuncCall("text_le", {lhs, rhs});
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

    if( bin->opcode >= Operation::Eq && bin->opcode <= Operation::Or )
        ret = builder.CreateSelect(ret, One, Zero);
		
    return ret;
}

///
llvm::Value* IrEmitter::emit(UnaryPtr un)
{
    // գեներացնել ենթաարտահայտությունը
    auto val = emit(un->subexpr);

    // ունար մինուս (բացասում)
    if( Operation::Sub == un->opcode )
        return builder.CreateFNeg(val, "neg");

    // ժխտում
    if( Operation::Not == un->opcode ) {
	  auto _cbv = builder.CreateFCmpUNE(val, Zero);
	  return builder.CreateSelect(_cbv, One, Zero);
	}
    
    return val;
}

///
void IrEmitter::setCurrentBlock(llvm::Function* fun, llvm::BasicBlock* bl)
{
    auto _cbb = builder.GetInsertBlock();

    if( nullptr != _cbb && nullptr == _cbb->getTerminator() )
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
    auto _V = builder.getVoidTy();
    auto _B = builder.getInt1Ty();
    auto _N = builder.getDoubleTy();
    auto _T = builder.getInt8PtrTy();

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
    auto ty_ep = llvm::FunctionType::get(builder.getInt32Ty(), {}, false);
    auto lk_ep = llvm::GlobalValue::ExternalLinkage;
    auto fc_ep = llvm::Function::Create(ty_ep, lk_ep, "main", module.get());

    auto _st  = llvm::BasicBlock::Create(context, "start", fc_ep);
    builder.SetInsertPoint(_st);

    // եթե ծրագրավորողը սահմանել է Main անունով ենթածրագիր, ապա
    // main()-ի մեջ կանչել այն, հակառակ դեպքում main-ը դատարկ է
    auto _Main = module->getFunction("Main");
    if( nullptr != _Main )
        builder.CreateCall(_Main, {});
    
    auto rv_ep = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
    builder.CreateRet(rv_ep);
}        

///
void IrEmitter::declareSubroutines(ProgramPtr prog)
{
    for( auto& subr : prog->members ) {
        // պարամետրերի տիպերի ցուցակի կառուցումը
        std::vector<llvm::Type*> ptypes;
        for( auto& pr : subr->parameters )
            ptypes.push_back(llvmType(typeOf(pr)));

        // վերադարձվող արժեքի տիպը
        llvm::Type* rtype = subr->hasValue ? 
                llvmType(typeOf(subr->name)) :
                builder.getVoidTy();

        // ստեղծել ֆունկցիայի հայտարարությունը
        auto functy = llvm::FunctionType::get(rtype, ptypes, false);
        auto linkage = llvm::GlobalValue::ExternalLinkage;
        llvm::Function::Create(functy, linkage, subr->name, module.get());
    }
}

///
void IrEmitter::defineSubroutines(ProgramPtr prog)
{
    for( auto& subr : prog->members )
        if( !subr->isBuiltIn )
            emit(subr);
}

///
llvm::Type* IrEmitter::llvmType(Type type)
{
    if( Type::Boolean == type )
        return BooleanType;

    if( Type::Numeric == type )
        return NumberType;

    if( Type::Textual == type )
        return TextType;

    return VoidType;
}

///
bool IrEmitter::createsTempText(ExpressionPtr expr)
{
    // թվային արտահայտությունը ժամանակավոր օբյեկտ չի ստեղծում
    if( Type::Numeric == expr->type )
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
