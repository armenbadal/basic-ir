
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
    module = std::make_unique<llvm::Module>(prog->filename, context);

    declareLibrary();

    for( auto& si : prog->members )
        emitSubroutine(si);

	// TODO: աշխատեցնել verify pass

    module->print(llvm::errs(), nullptr);
    //module->print(mOut, nullptr);
}

//
void IrEmitter::emitSubroutine( SubroutinePtr subr )
{
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

    // ֆունկցիայի տիպը
    auto procty = llvm::FunctionType::get(rtype, ptypes, false);
    // ֆունկցիա օբյեկտը
    auto fun = llvm::Function::Create(procty,
        llvm::GlobalValue::ExternalLinkage,
        subr->name, module.get());

    // եթե սա ներդրված ենթածրագիր է, ապա գեներացնում
    // ենք միայն հայտարարությունը
    if( subr->isBuiltIn )
        return;

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
    for( auto& arg : fun->args() )
        if( arg.getType()->isPointerTy() ) {
            auto parval = builder.CreateCall(library["text_clone"], { &arg });
            builder.CreateStore(parval, varaddresses[arg.getName()]);
            localtexts.remove(varaddresses[arg.getName()]);
        }
        else
            builder.CreateStore(&arg, varaddresses[arg.getName()]);

    // տեքստային օբյեկտների համար գեներացնել սկզբնական արժեք
    // (սա արվում է վերագրման ժամանակ հին արժեքը ջնջելու և 
    // նորը վերագրելու սիմետրիկությունն ապահովելու համար)
    auto one = builder.getInt64(1);
    for( auto vp : localtexts ) {
        auto deva = builder.CreateCall(library["malloc"], { one });
        builder.CreateStore(deva, vp);
    }

    // գեներացնել ֆունկցիայի մարմինը
    emitSequence(std::dynamic_pointer_cast<Sequence>(subr->body));

    // ազատել տեքստային օբյեկտների զբաղեցրած հիշողությունը
    for( auto vi : subr->locals ) {
        if( Type::Number == vi->type )
            continue;
        if( vi->name == subr->name )
            continue;
        auto addr = varaddresses[vi->name];
        auto deva = builder.CreateCall(library["free"], { addr });
    }

    // վերադարձվող արժեք
    if( rtype->isVoidTy() )
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
        auto e0 = builder.CreateCall(library["text_clone"], {val});
        builder.CreateCall(library["free"], addr);
        builder.CreateStore(e0, addr);
        if( val->getName().startswith("_temp_") )
            builder.CreateCall(library["free"], val);
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
        case Operation::Conc:
            ret = builder.CreateCall(library["text_concatenate"], {lhs, rhs}, "_temp_");
            break;
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
void IrEmitter::declareLibSubr( const std::string& name,
    llvm::ArrayRef<llvm::Type*> patys, llvm::Type* rty )
{
    auto functy = llvm::FunctionType::get(rty, patys, false);
    library[name] = llvm::Function::Create(functy, 
        llvm::GlobalValue::ExternalLinkage, name, module.get());
}

/**/
void IrEmitter::declareLibrary()
{
    auto _V = builder.getVoidTy();
    auto _N = builder.getDoubleTy();
    auto _T = builder.getInt8PtrTy();

    declareLibSubr("text_clone", {_T}, _T);
    declareLibSubr("text_input", {}, _T);
    declareLibSubr("text_print", {_T}, _V);
    declareLibSubr("text_concatenate", {_T, _T}, _T);

    declareLibSubr("number_input", {}, _N);
    declareLibSubr("number_print", {_N}, _V);

    declareLibSubr("malloc", { builder.getInt64Ty() }, _T);
    declareLibSubr("free", {_T}, _V);
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
