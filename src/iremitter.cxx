#include "iremitter.hxx"
#include "ast.hxx"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Pass.h>

#include <list>
#include <system_error>
#include <utility>
#include <vector>

#include <iostream>

namespace basic {
///
IrEmitter::IrEmitter(llvm::LLVMContext& cx, llvm::Module& md)
    : context{cx}, moduler{md}, builder{context}
{
}

///
bool IrEmitter::emitFor(ProgramPtr prog)
{
    try {
        visit(prog);
        llvm::verifyModule(moduler);
    }
    catch(...) {
        return false;
    }

    return true;
}

///
void IrEmitter::visit(ProgramPtr prog)
{
    // նախապատրաստել գրադարանային (սպասարկող) ֆունկցիաները
    prepareLibrary();

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
}

//
void IrEmitter::visit(SubroutinePtr subr)
{
    // մոդուլից վերցնել ֆունկցիայի հայտարարությունը դրան
    // մարմին ավելացնելու համար
    auto* func = moduler.getFunction(subr->name);

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
    visit(subr->body);

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
void IrEmitter::visit(StatementPtr st)
{
    dispatch(st);
}

///
void IrEmitter::visit(SequencePtr seq)
{
    for( auto& st : seq->items )
        visit(st);
}

///
void IrEmitter::visit(LetPtr let)
{
    auto* val = visit(let->expr);
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
void IrEmitter::visit(InputPtr inp)
{
    // ստանալ հրավերքի տեքստի հասցեն
    auto* prompt = visit(inp->prompt);

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
void IrEmitter::visit(PrintPtr pri)
{
    // արտածվող արտահայտության կոդը
    auto* expr = visit(pri->expr);
    
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
void IrEmitter::visit(IfPtr sif)
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
        auto* cnd = visit(ifp->condition);
		//cnd = builder.CreateFCmpUNE(cnd, Zero);
        
        // անցում ըստ պայմանի
        builder.CreateCondBr(cnd, thenBlock, elseBlock);

        // then-ի հրամաններ
        setCurrentBlock(func, thenBlock);

        visit(ifp->decision);
        builder.CreateBr(endIf);

        // պատրաստվել հաջորդ բլոկին
        setCurrentBlock(func, elseBlock);
        
        // հաջորդ բլոկի մշակում
        sp = ifp->alternative;
    }
    
    // կա՞ արդյոք else-բլոկ
    if( nullptr != sp )
        visit(sp);
    
    setCurrentBlock(func, endIf);
}

///
void IrEmitter::visit(WhilePtr swhi)
{
    // ընթացիկ ֆունկցիան
    auto* func = builder.GetInsertBlock()->getParent();

    // ցիկլի պայմանի, մարմնի և ավարտի բլոկները
    auto* condBlock = llvm::BasicBlock::Create(context, "", func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, "", func);
    auto* endWhile = llvm::BasicBlock::Create(context, "", func);

    setCurrentBlock(func, condBlock);

    // գեներացնել կրկնման պայմանը
    auto* condEx = visit(swhi->condition);
    builder.CreateCondBr(condEx, bodyBlock, endWhile);

    setCurrentBlock(func, bodyBlock);

    // գեներացնել ցիկլի մարմինը
    visit(swhi->body);
    builder.CreateBr(condBlock);

    setCurrentBlock(func, endWhile);
}

///
void IrEmitter::visit(ForPtr sfor)
{
    // ընթացիկ ֆունկցիան
    auto* func = builder.GetInsertBlock()->getParent();

    auto* condBlock = llvm::BasicBlock::Create(context, "", func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, "", func);
    auto* endFor = llvm::BasicBlock::Create(context, "", func);

    // ցիկլի պարամետրի հասցեն
    auto* param = varAddresses[sfor->parameter->name];
    // գեներացնել սկզբնական արժեքի արտահայտությունը
    auto* init = visit(sfor->begin);
    // պարամետրին վերագրել սկզբնական արժեքը
    builder.CreateStore(init, param);
    // գեներացնել վերջնական արժեքի արտահայտությունը
    auto* finish = visit(sfor->end);
    // քայլը հաստատուն թիվ է
    auto* step = llvm::ConstantFP::get(NumericTy, sfor->step->value);
    
    setCurrentBlock(func, condBlock);

    // եթե պարամետրի արժեքը >= (կամ <=, եթե քայլը բացասական է)
    // վերջնականից, ապա ավարտել ցիկլը
    auto* parVal = builder.CreateLoad(NumericTy, param);
    llvm::Value* coex = nullptr;
    if( sfor->step->value > 0.0 )
        coex = builder.CreateFCmpOLT(parVal, finish);
    else if( sfor->step->value < 0.0 )
        coex = builder.CreateFCmpOGT(parVal, finish);
    builder.CreateCondBr(coex, bodyBlock, endFor);

    setCurrentBlock(func, bodyBlock);

    // գեներացնել մարմինը
    visit(sfor->body);

    // պարամետրի արժեքին գումարել քայլի արժեքը
    auto* parval = builder.CreateLoad(NumericTy, param);
    auto* nwpv = builder.CreateFAdd(parval, step);
    builder.CreateStore(nwpv, param);

    // կրկնել ցիկլը
    builder.CreateBr(condBlock);

    setCurrentBlock(func, endFor);
}

///
void IrEmitter::visit(CallPtr cal)
{
    // պրոցեդուրայի կանչը նույն ֆունկցիայի կիրառումն է
    visit(cal->subrCall);
}

///
llvm::Value* IrEmitter::visit(ExpressionPtr expr)
{
    return dispatch(expr);
}

///
llvm::Value* IrEmitter::visit(TextPtr txt)
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
llvm::Value* IrEmitter::visit(NumberPtr num)
{
    // գեներացնել թվային հաստատուն
    return llvm::ConstantFP::get(NumericTy, num->value);
}

///
llvm::Value* IrEmitter::visit(BooleanPtr bv)
{
    // գեներացնել տրամաբանական հաստատուն
    return llvm::ConstantInt::getBool(BooleanTy, bv->value);
}

///
llvm::Value* IrEmitter::visit(VariablePtr var)
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
llvm::Value* IrEmitter::visit(ApplyPtr apy)
{
    // գեներացնել կանչի արգումենտները
    llvm::SmallVector<llvm::Value*> argus, temps;
    for( const auto& ai : apy->arguments ) {
        auto ap = visit(ai);
        argus.push_back(ap);
        if( createsTempText(ai) )
            temps.push_back(ap);
    }

    // կանչել ֆունկցիան ու պահել արժեքը
	auto callee = userFunction(apy->callee->name);
    auto* calv = builder.CreateCall(callee, argus);

    // մաքրել կանչի ժամանակավոր արգումենտները
    for( auto* ai : temps )
        if( ai->getType()->isPointerTy() )
            createLibraryFuncCall("free", { ai });

    // վերադարձնել կանչի արդյունքը
    return calv;
}

///
llvm::Value* IrEmitter::visit(BinaryPtr bin)
{
    const bool textuals = bin->left->is(Type::Textual)
                       && bin->right->is(Type::Textual);
    const bool numerics = bin->left->is(Type::Numeric)
                       && bin->right->is(Type::Numeric);
    const bool booleans = bin->left->is(Type::Boolean)
                       && bin->right->is(Type::Boolean);

    auto* lhs = visit(bin->left);
    auto* rhs = visit(bin->right);
	
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
llvm::Value* IrEmitter::visit(UnaryPtr un)
{
    // գեներացնել ենթաարտահայտությունը
    auto* val = visit(un->subexpr);

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
    // տեքստային ֆունկցիաներ
    declareLibraryFunction("text_clone", "T(T)");
    declareLibraryFunction("text_input", "T(T)");
    declareLibraryFunction("text_print", "V(T)");
    declareLibraryFunction("text_conc", "T(TT)");
    declareLibraryFunction("text_mid", "T(TNN)");
    declareLibraryFunction("text_str", "T(N)");
    declareLibraryFunction("text_eq", "B(TT)");
    declareLibraryFunction("text_ne", "B(TT)");
    declareLibraryFunction("text_gt", "B(TT)");
    declareLibraryFunction("text_ge", "B(TT)");
    declareLibraryFunction("text_lt", "B(TT)");
    declareLibraryFunction("text_le", "B(TT)");

    // թվային ֆունկցիաներ
    declareLibraryFunction("number_input", "N(T)");
    declareLibraryFunction("number_print", "V(N)");

    // մաթեմատիկական ֆունկցիաներ
    declareLibraryFunction("pow", "N(NN)");
    declareLibraryFunction("sqrt", "N(N)");

    // հիշողության ֆունկցիաներ
    library["malloc"] = llvm::FunctionType::get(
            builder.getInt8PtrTy(), {builder.getInt64Ty()}, false);
    library["free"] = llvm::FunctionType::get(
            VoidTy, {builder.getInt8PtrTy()}, false);
}

///
void IrEmitter::declareLibraryFunction(std::string_view name, std::string_view signature)
{
    auto* returnType = llvmType(static_cast<Type>(signature[0]));

    signature.remove_prefix(2); // drop return type and '('
    signature.remove_suffix(1); // drop ')'

    llvm::SmallVector<llvm::Type*> paramTypes;
    for( const char t : signature )
        paramTypes.push_back(llvmType(static_cast<Type>(t)));

    library[std::string{name}] = llvm::FunctionType::get(returnType, paramTypes, false);
}

///
llvm::FunctionCallee IrEmitter::libraryFunction(std::string_view name)
{
    return moduler.getOrInsertFunction(name, library[std::string{name}]);
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
	
    return moduler.getFunction(name);
}

///
void IrEmitter::createEntryPoint()
{
    auto* Int32Ty = builder.getInt32Ty();

    auto* mainType = llvm::FunctionType::get(Int32Ty, {}, false);
    const auto linkage = llvm::GlobalValue::ExternalLinkage;
    auto* mainFunc = llvm::Function::Create(mainType, linkage, "main", &moduler);

    auto* start  = llvm::BasicBlock::Create(context, "start", mainFunc);
    builder.SetInsertPoint(start);

    // եթե ծրագրավորողը սահմանել է Main անունով ենթածրագիր, ապա
    // main()-ի մեջ կանչել այն, հակառակ դեպքում main-ը դատարկ է
    if( auto* udMain = moduler.getFunction("Main"); nullptr != udMain )
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
        llvm::Function::Create(funcType, linkage, subr->name, &moduler);
    }
}

///
void IrEmitter::defineSubroutines(ProgramPtr prog)
{
    for( const auto& subr : prog->members )
        if( !subr->isBuiltIn )
            visit(subr);
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
