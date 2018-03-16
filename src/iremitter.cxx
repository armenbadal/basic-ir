
#include "iremitter.hxx"
#include "ast.hxx"

#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>
#include <list>

/*
// աջակցող գրադարանը բեռնելու համար

std::unique_ptr<Module> llvm::parseAssemblyString(
    StringRef AsmString,
    SMDiagnostic& Error,
    LLVMContext& Context,
    SlotMapping* Slots = nullptr,
    bool UpgradeDebugInfo = true,
    StringRef DataLayoutString = "");

սtd::unique_ptr<Module> llvm::parseAssemblyFile(
    StringRef Filename,
    SMDiagnostic& Error,
    LLVMContext& Context,
    SlotMapping* Slots = nullptr,
    bool UpgradeDebugInfo = true,
    StringRef DataLayoutString = "");

// llvm::Linker դասի օգնությամբ կապակցել գրադարանի ու ծրագրի մոդուլները
*/


namespace basic {
///
bool IrEmitter::emitIrCode( ProgramPtr prog )
{
    emitProgram(prog);
    return true;
}

///
void IrEmitter::emitProgram( ProgramPtr prog )
{
    // ստեղծել LLVM-ի Module օբյեկտ՝ դրա հասցեն պահելով
    // STL գրադարանի unique_ptr-ի մեջ։
    module = std::make_unique<llvm::Module>(prog->filename, context);

    // հայտարարել գրադարանային (արտաքին) ֆունկցիաները
    declareLibrary();

    // սեփական ֆունկցիաների հայտարարությունն ու սահմանումը
    // հարմար է առանձնացնել, որպեսզի Apply և Call գործողությունների
    // գեներացիայի ժամանակ արդեն գոյություն ունենան LLVM-ի
    // Function օբյեկտները
    
    // հայտարարել սեփական ֆունկցիաները
    declareSubroutines(prog);
    //  սահմանել սեփական ֆունկցիաները
    defineSubroutines(prog);

    // TODO: գեներացնել արտաքին main() ֆունկցիա, 
    // կամ Main անունով ֆունկցիան անվանափոխել ու
    // դարձնել մուտքի կետ

    // TODO: աշխատեցնել verify pass մոդուլի համար

    module->print(llvm::errs(), nullptr);
    //module->print(mOut, nullptr);
}

//
void IrEmitter::emitSubroutine( SubroutinePtr subr )
{
    // մոդուլից վերցնել ֆունկցիայի հայտարարությունը դրան
    // մարմին ավելացնելու համար
    auto fun = module->getFunction(subr->name);

    // TODO: քանի որ նախ գեներացվելու են ֆունկցիաների 
    // հայտարարությունները, ապա նույն այդ ցուցակով 
    // ամեն մի ֆունկցիայի համար գեներացվելու է մարմին,
    // համարյա բացառված է, որ fun ցուցիչը զրոյական լինի,
    // սակայն, կոդի ճիշտ կազմակերպվածության տեսակետից,
    // ճիշտ կլինի, որ այս և սրա նման դեպքերում աշխատանքը
    // շարունակվի ցուցիչի ոչ զրոյական լինելը ստուգելուց հետո

    // ֆունկցիայի առաջին պիտակը (ցույց է տալիս ֆունկցիայի սկիզբը)
    auto start = llvm::BasicBlock::Create(context, "start", fun);
    builder.SetInsertPoint(start);

    // ֆունկցիայի պարամետրերին տալ սահմանված անունները
    for( auto& arg : fun->args() ) {
        int ix = arg.getArgNo();
        arg.setName(subr->parameters[ix]);
    }

    // մաքրել varaddresses ցուցակը
    varaddresses.clear();

    // տեքստային օբյեկտների հասցեները
    std::list<llvm::Value*> localtexts;
    
    // բոլոր լոկալ փոփոխականների, պարամետրերի 
    // և վերադարձվող արժեքի համար
    for( auto& vi : subr->locals ) {
        auto vty = llvmType(vi->type);
        auto addr = builder.CreateAlloca(vty, nullptr, vi->name + "_addr");
        varaddresses[vi->name] = addr;
        if( Type::Text == vi->type )
            localtexts.push_back(addr);
    }

    // պարամետրերի արժեքները վերագրել լոկալ օբյեկտներին
    auto text_clone_f = module->getFunction("text_clone");
    for( auto& arg : fun->args() )
        if( arg.getType()->isPointerTy() ) {
            auto parval = builder.CreateCall(text_clone_f, { &arg });
            builder.CreateStore(parval, varaddresses[arg.getName()]);
            localtexts.remove(varaddresses[arg.getName()]);
        }
        else
            builder.CreateStore(&arg, varaddresses[arg.getName()]);

    // տեքստային օբյեկտների համար գեներացնել սկզբնական արժեք
    // (սա արվում է վերագրման ժամանակ հին արժեքը ջնջելու և 
    // նորը վերագրելու սիմետրիկությունն ապահովելու համար)
    auto one = builder.getInt64(1);
    auto malloc_f = module->getFunction("malloc");
    for( auto vp : localtexts ) {
        auto deva = builder.CreateCall(malloc_f, { one });
        builder.CreateStore(deva, vp);
    }

    // գեներացնել ենթածրագրի մարմնի հրամանները
    emitSequence(std::dynamic_pointer_cast<Sequence>(subr->body));

    // ազատել տեքստային օբյեկտների զբաղեցրած հիշողությունը
    // Յուրաքանչյուր ֆունկցիայի ավարտին պետք է ազատել
    // տեքստային օբյեկտների զբաղեցրած հիշողությունը։ 
    // Բացառություն պիտի լինի միայն ֆունկցիաքյի անունով 
    // փոփոխականին կապված արժեքը, որը վերադարձվելու է
    // ֆունկցիային կանչողին
    auto free_f = module->getFunction("free");
    for( auto vi : subr->locals ) {
        if( Type::Number == vi->type )
            continue;
        if( vi->name == subr->name )
            continue;
        auto addr = varaddresses[vi->name];
        auto deva = builder.CreateCall(free_f, { addr });
    }

    // վերադարձվող արժեք
    if( fun->getReturnType()->isVoidTy() )
        builder.CreateRetVoid();
    else {
        auto rv = builder.CreateLoad(varaddresses[subr->name]);
        builder.CreateRet(rv);
    }
}

///
void IrEmitter::emitSequence( SequencePtr seq )
{
    for( auto& st : seq->items ) {
        switch( st->kind ) {
            case NodeKind::Let:
                emitLet(std::dynamic_pointer_cast<Let>(st));
                break;
            case NodeKind::Input:
                emitInput(std::dynamic_pointer_cast<Input>(st));
                break;
            case NodeKind::Print:
                emitPrint(std::dynamic_pointer_cast<Print>(st));
                break;
            case NodeKind::If:
                break;
            case NodeKind::While:
                break;
            case NodeKind::For:
                break;
            case NodeKind::Call:
                break;
            default:
                break;
        }
    }
}

///
void IrEmitter::emitLet( LetPtr let )
{
    auto val = emitExpression(let->expr);
    auto addr = varaddresses[let->varptr->name];
    if( Type::Text == let->varptr->type ) {
        // TODO: եթե վերագրման աջ կողմում ֆունկցիայի կանչ է կամ
        // տեքստերի կցման `&` գործողություն, ապա ոչ թե պատճենել
        // ժամանակավոր օբյեկտը, այլ միանգամից օգտագործել այն

        // TODO: տեքստի պատճենումը կատարել միայն տեքստային լիտերալներից
        // կամ այլ տեքստային փոփոխականներից
        auto text_clone_f = module->getFunction("text_clone");
        auto free_f = module->getFunction("free");
        
        auto e0 = builder.CreateCall(text_clone_f, {val});
        builder.CreateCall(free_f, addr);
        builder.CreateStore(e0, addr);
        if( val->getName().startswith("_temp_") )
            builder.CreateCall(free_f, val);
    }
    else
        builder.CreateStore(val, addr);
}

///
void IrEmitter::emitInput( InputPtr inp )
{
    // կանչել գրադարանային ֆունկցիա
    // input_text() կամ input_number()
}

///
void IrEmitter::emitPrint( PrintPtr pri )
{
    // կանչել գրադարանային ֆունկցիա
    // print_text() կամ print_number()
}

/*
//
void IrEmitter::emitIf(If* ifSt, llvm::BasicBlock* endBB)
{
    if (getEmittedNode(ifSt)) {
        return;
    }
    auto insertBB = mBuilder.GetInsertBlock();
    auto fun = insertBB->getParent();
    auto cnd = emitExpression(ifSt->condition);

    llvm::BasicBlock* decBB = llvm::BasicBlock::Create(llvmContext, "bb", fun, endBB);
    mBuilder.SetInsertPoint(decBB);
    emitStatement(ifSt->decision, endBB);

    llvm::BasicBlock* altBB = endBB;
    if (ifSt->alternative) {
        altBB = llvm::BasicBlock::Create(llvmContext, "bb", fun, endBB);
        mBuilder.SetInsertPoint(altBB);
        emitStatement(ifSt->alternative, endBB);
    }

    mBuilder.SetInsertPoint(insertBB);
    auto br = mBuilder.CreateCondBr(cnd, decBB, altBB);

    if (!decBB->getTerminator()) {
        mBuilder.SetInsertPoint(decBB);
        mBuilder.CreateBr(endBB);
    }

    if (!altBB->getTerminator()) {
        mBuilder.SetInsertPoint(altBB);
        mBuilder.CreateBr(endBB);
    }

    mBuilder.SetInsertPoint(endBB);
    mEmittedNodes.insert({ ifSt, br });
}

void IrEmitter::emitWhile(While* whileSt, llvm::BasicBlock* endBB)
{
    llvm::BasicBlock* head = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);

    mBuilder.CreateBr(head);

    mBuilder.SetInsertPoint(head);
    auto cnd = emitExpression(whileSt->condition);
    auto br = mBuilder.CreateCondBr(cnd, body, endBB);

    mBuilder.SetInsertPoint(body);
    emitStatement(whileSt->body, endBB);

    if (!body->getTerminator()) {
        mBuilder.SetInsertPoint(body);
        mBuilder.CreateBr(head);
    }
    mBuilder.SetInsertPoint(endBB);
}
*/

//
void IrEmitter::emitFor( ForPtr sfor )
{
    // TODO:
    // 1. գեներացնել սկզբնական արժեքի արտահայտությունը,
    // 2. գեներացնել վերջնական արժեքի արտահայտությունը,
    // 3. պարամետրին վերագրել սկզբնական արժեքը,
    // 4. եթե պարամետրի արժեքը >= (կամ <=, եթե քայլը բացասական է) վերջնականից,
    // 5. ապա դուրս գալ ցիկլից,
    // 6. գեներացնել մարմինը,
    // 7. պարամետրի արժեքին գումարել քայլի արժեքը,
    // 8. շարունակել 4-րդ կետից։

    /*
    llvm::BasicBlock* head = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* exit = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);

    auto param_addr = getVariableAddress(forSt->parameter->name);
    auto begin = emitExpression(forSt->begin);
    mBuilder.CreateStore(begin, param_addr);

    //Setting step 1 by default
    llvm::Value* step = mBuilder.getInt32(1);

    //Looking if step is given
    if (forSt->step) {
        step = emitExpression(forSt->step);
    }

    auto end = emitExpression(forSt->end);
    mBuilder.CreateBr(head);

    mBuilder.SetInsertPoint(head);
    auto param = mBuilder.CreateLoad(param_addr);
    auto cnd = mBuilder.CreateFCmpOLE(param, end, "le");
    mBuilder.CreateCondBr(cnd, body, endBB);

    //Handling the body
    mBuilder.SetInsertPoint(body);
    emitStatement(forSt->body, exit);

    //Incrementing the index
    auto inc_param = mBuilder.CreateFAdd(param, step);
    mBuilder.CreateStore(inc_param, param_addr);

    if (!body->getTerminator()) {
        mBuilder.SetInsertPoint(body);
        mBuilder.CreateBr(head);
    }

    mBuilder.SetInsertPoint(endBB);
    */
}

///
void IrEmitter::emitCall( CallPtr cal )
{
}

///
llvm::Value* IrEmitter::emitExpression( ExpressionPtr expr )
{
    llvm::Value* res = nullptr;

    switch( expr->kind ) {
        case NodeKind::Number:
            res = emitNumber(std::dynamic_pointer_cast<Number>(expr));
            break;
        case NodeKind::Text:
            res = emitText(std::dynamic_pointer_cast<Text>(expr));
            break;
        case NodeKind::Variable:
            res = emitLoad(std::dynamic_pointer_cast<Variable>(expr));
            break;
        case NodeKind::Unary:
            break;
        case NodeKind::Binary:
            res = emitBinary(std::dynamic_pointer_cast<Binary>(expr));
            break;
        case NodeKind::Apply:
            res = emitApply(std::dynamic_pointer_cast<Apply>(expr));
            break;
        default:
            break;
    }

    return res;
}

//
llvm::Value* IrEmitter::emitText( TextPtr txt )
{
    // եթե տրված արժեքով տող արդեն սահմանված է գլոբալ
    // տիրույթում, ապա վերադարձնել դրա հասցեն
    auto sri = globaltexts.find(txt->value);
    if( sri != globaltexts.end() )
        return sri->second;

    // ... հակառակ դեպքում՝ սահմանել նոր գլոբալ տող, դրա հասցեն
    // պահել գլոբալ տողերի ցուցակում և վերադարձնել որպես արժեք
    auto strp = builder.CreateGlobalStringPtr(txt->value, "g_str");
    globaltexts[txt->value] = strp;

    return strp;
}

//
llvm::Constant* IrEmitter::emitNumber( NumberPtr num )
{
    return llvm::ConstantFP::get(builder.getDoubleTy(), num->value);
}

///
llvm::LoadInst* IrEmitter::emitLoad( VariablePtr var )
{
    llvm::Value* vaddr = varaddresses[var->name];
    return builder.CreateLoad(vaddr, var->name);
}

///
llvm::Value* IrEmitter::emitApply( ApplyPtr apy )
{
    // գեներացնել կանչի արգումենտները
    std::vector<llvm::Value*> argus;
    for( auto& ai : apy->arguments ) {
        auto ap = emitExpression(ai);
        argus.push_back(ap);
    }

    // կանչել ֆունկցիան ու պահել արժեքը
    auto callee = module->getFunction(apy->procptr->name);
    auto calv = builder.CreateCall(callee, argus, "_temp_");

    // մաքրել կանչի ժամանակավոր արգումենտները
    auto free_f = module->getFunction("free");
    for( auto ai : argus )
        if( ai->getType()->isPointerTy() )
            builder.CreateCall(free_f, { ai });

        // վերադարձնել կանչի արդյունքը
    return calv;
}

/**/
llvm::Value* IrEmitter::emitBinary( BinaryPtr bin )
{
    llvm::Value* lhs = emitExpression(bin->subexpro);
    llvm::Value* rhs = emitExpression(bin->subexpri);

    llvm::Value* ret = nullptr;
    switch (bin->opcode) {
        /*
        case Operation::None:
            break;
        case Operation::Add:
            ret = mBuilder.CreateFAdd(lhs, rhs, "add");
            break;
        case Operation::Sub:
            ret = mBuilder.CreateFSub(lhs, rhs, "sub");
            break;
        case Operation::Mul:
            ret = mBuilder.CreateFMul(lhs, rhs, "mul");
            break;
        case Operation::Div:
            ret = mBuilder.CreateFDiv(lhs, rhs, "div");
            break;
        case Operation::Mod:
            ret = mBuilder.CreateFRem(lhs, rhs, "rem");
            break;
        case Operation::Pow:
            assert("POW operator is not handled yet");
            break;
        case Operation::Eq:
            ret = mBuilder.CreateFCmpOEQ(lhs, rhs, "eq");
            break;
        case Operation::Ne:
            ret = mBuilder.CreateFCmpONE(lhs, rhs, "ne");
            break;
        case Operation::Gt:
            ret = mBuilder.CreateFCmpOGT(lhs, rhs, "ne");
            break;
        case Operation::Ge:
            ret = mBuilder.CreateFCmpOGE(lhs, rhs, "ge");
            break;
        case Operation::Lt:
            ret = mBuilder.CreateFCmpOLT(lhs, rhs, "lt");
            break;
        case Operation::Le:
            ret = mBuilder.CreateFCmpOLE(lhs, rhs, "le");
            break;
        case Operation::And:
            ret = mBuilder.CreateAnd(lhs, rhs, "and");
            break;
        case Operation::Or:
            ret = mBuilder.CreateOr(lhs, rhs, "or");
            break;
        */
        case Operation::Conc: {
            auto text_concatenate_f = module->getFunction("text_concatenate");
            ret = builder.CreateCall(text_concatenate_f, {lhs, rhs}, "_temp_");
            break;
        }
        default:
            break;
    }

    return ret;
}

/*
llvm::Value* IrEmitter::emitUnary(Unary* un)
{
    llvm::Value* val = emitExpression(un->subexpr);
    switch (un->opcode) {
        case Operation::Sub:
            return mBuilder.CreateFNeg(val, "neg");
        case Operation::Not:
            return mBuilder.CreateNot(val, "not");
        default: {
            assert("Invalid unary operation");
        }
    }
    return nullptr;
}
*/

/**/
void IrEmitter::declareFunction( const String& name,
    const TypeVector& patys, IrType* rty, bool external )
{
    auto functy = llvm::FunctionType::get(rty, patys, false);
    auto linkage = external ? llvm::GlobalValue::ExternalLinkage :
        llvm::GlobalValue::InternalLinkage;
    llvm::Function::Create(functy, linkage, name, module.get());
}

/**/
void IrEmitter::declareLibrary()
{
    auto _V = builder.getVoidTy();
    auto _N = builder.getDoubleTy();
    auto _T = builder.getInt8PtrTy();

    declareFunction("text_clone", {_T}, _T, true);
    declareFunction("text_input", {}, _T, true);
    declareFunction("text_print", {_T}, _V, true);
    declareFunction("text_concatenate", {_T, _T}, _T, true);

    declareFunction("number_input", {}, _N, true);
    declareFunction("number_print", {_N}, _V, true);

    declareFunction("malloc", { builder.getInt64Ty() }, _T, true);
    declareFunction("free", {_T}, _V, true);
}

/**/
void IrEmitter::declareSubroutines( ProgramPtr prog )
{
    for( auto& subr : prog->members ) {
        // պարամետրերի տիպերի ցուցակի կառուցումը
        std::vector<llvm::Type*> ptypes;
        for( auto& pr : subr->parameters )
            ptypes.push_back(llvmType(typeOf(pr)));

        // վերադարձվող արժեքի տիպը
        llvm::Type* rtype = nullptr;
        if( subr->hasValue )
            rtype = llvmType(typeOf(subr->name));
        else
            rtype = builder.getVoidTy();

        // ստեղծել ֆունկցիայի հայտարարությունը
        declareFunction(subr->name, ptypes, rtype);
    }
}

/**/
void IrEmitter::defineSubroutines( ProgramPtr prog )
{
    for( auto& subr : prog->members )
        if( !subr->isBuiltIn )
            emitSubroutine(subr);
}

/**/
llvm::Type* IrEmitter::llvmType( Type type )
{
    if( Type::Number == type )
        return builder.getDoubleTy();

    if( Type::Text == type )
        return builder.getInt8PtrTy();

    return builder.getVoidTy();
}
} // namespace llvm
