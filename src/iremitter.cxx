
#include "iremitter.hxx"
#include "ast.hxx"

#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>

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
bool IrEmitter::emitIrCode( Program* prog )
{
    emitProgram(prog);
}

///
void IrEmitter::emitProgram( Program* prog )
{
    module = new llvm::Module(prog->filename, context);

    for( Subroutine* si : prog->members )
        emitSubroutine(si);

    module->print(llvm::errs(), nullptr);
    //module->print(mOut, nullptr);
}

//
void IrEmitter::emitSubroutine( Subroutine* subr )
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
    auto fun = llvm::Function::Create(procty, llvm::GlobalValue::ExternalLinkage, subr->name, module);

    // եթե սա ներդրված ենթածրագիր է, ապա գեներացնում ենք միայն հայտարարությունը
    if( subr->isBuiltIn )
        return;

    // ֆունկցիայի առաջին պիտակը (ցույց է տալիս ֆունկցիայի սկիզբը)
    auto start = llvm::BasicBlock::Create(context, "start", fun);
    builder.SetInsertPoint(start);

    // ֆունկցիայի պարամետրերին տալ անուններ
    for( auto& arg : fun->args() ) {
        int ix = arg.getArgNo();
        arg.setName(subr->parameters[ix]);
    }

    // օբյեկտներ բոլոր լոկալ փոփոխականների, պարամետրերի 
    // և վերադարձվող արժեքի համար
    for( Variable* vi : subr->locals ) {
        auto vty = llvmType(vi->type);
        auto addr = builder.CreateAlloca(vty, nullptr, vi->name + "_addr");
        varaddresses[vi->name] = addr;
    }

    // պարամետրերի արժեքները վերագրել լոկալ օբյեկտներին
    for( auto& arg : fun->args() )
        builder.CreateStore(&arg, varaddresses[arg.getName()]);

    // գեներացնել ֆունկցիայի մարմինը
    emitSequence(dynamic_cast<Sequence*>(subr->body));

    // լրացնել, ուղղել
    if( rtype->isVoidTy() )
        builder.CreateRetVoid();
    else {
        auto rv = builder.CreateLoad(varaddresses[subr->name]);
        builder.CreateRet(rv);
    }
}

///
void IrEmitter::emitSequence( Sequence* seq )
{
    for( Statement* st : seq->items ) {
        switch( st->kind ) {
            case NodeKind::Let:
                emitLet(dynamic_cast<Let*>(st));
                break;
            case NodeKind::Input:
                emitInput(dynamic_cast<Input*>(st));
                break;
            case NodeKind::Print:
                emitPrint(dynamic_cast<Print*>(st));
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
void IrEmitter::emitLet( Let* let )
{
    // TODO: տողերի դեպքում՝ ուրիշ մոտեցում
    auto val = emitExpression(let->expr);
    auto addr = varaddresses[let->varptr->name];
    builder.CreateStore(val, addr);
}

///
void IrEmitter::emitInput( Input* inp )
{
    // կանչել գրադարանային ֆունկցիա
    // input_text() կամ input_number()
}

///
void IrEmitter::emitPrint( Print* pri )
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
void IrEmitter::emitFor( For* sfor )
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
llvm::Value* IrEmitter::emitExpression( Expression* expr )
{
    llvm::Value* res = nullptr;

    switch( expr->kind ) {
        case NodeKind::Number:
            res = emitNumber(dynamic_cast<Number*>(expr));
            break;
        case NodeKind::Text:
            res = emitText(dynamic_cast<Text*>(expr));
            break;
        case NodeKind::Variable:
            res = emitLoad(dynamic_cast<Variable*>(expr));
            break;
        case NodeKind::Unary:
            break;
        case NodeKind::Binary:
            break;
        case NodeKind::Apply:
            break;
        default:
            break;
    }

    return res;
}

//
llvm::Value* IrEmitter::emitText( Text* txt )
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
llvm::Constant* IrEmitter::emitNumber( Number* num )
{
    return llvm::ConstantFP::get(builder.getDoubleTy(), num->value);
}

///
llvm::LoadInst* IrEmitter::emitLoad( Variable* var )
{
    llvm::Value* vaddr = varaddresses[var->name];
    return builder.CreateLoad(vaddr, var->name);
}

/*
llvm::Value* IrEmitter::emitBinary(Binary* bin)
{
    if (auto r = getEmittedNode(bin)) {
        return r;
    }
    llvm::Value* lhs = emitExpression(bin->subexpro);
    assert(lhs);
    llvm::Value* rhs = emitExpression(bin->subexpri);
    assert(rhs);
    llvm::Value* ret = nullptr;
    switch (bin->opcode) {
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
        case Operation::Conc:
            // TODO: [18:02:36] Armen Badalian: դեռ չեմ պատկերացնում, թե տողերի կոնկատենացիայի համար ինչ կոդ ես գեներացնելու
            //[18:03:16] Tigran Sargsyan: ես էլ չեմ պատկերացնում
            //[18:03:21] Tigran Sargsyan: :)
            //[18:03:33] Tigran Sargsyan: բայց դե միբան կբստրենք
            //[18:03:44] Armen Badalian: միգուցե տողերը սարքենք հին Պասկալի պես, երկարությունը ֆիքսենք 255 նիշ, ու բոլոր գործողությունները դրանով անենք
            //[18:04:16 | Edited 18:04:20] Armen Badalian: հին Պասկալում տողի առաջին բայթում գրվում էր տողի երկարությունը
            //[18:04:30] Armen Badalian: ու դա կարող էր լինել 255
            //[18:05:14] Tigran Sargsyan: տարբերակ ա, կարելի ա մտածել
            assert("CONC operator is not handled yet");
            break;
        default: {
            assert("Undefined binary operator");
            break;
        }
    }
    mEmittedNodes.insert({ bin, ret });
    return ret;
}

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

llvm::Type* IrEmitter::llvmType( Type type )
{
    if( Type::Number == type )
        return builder.getDoubleTy();

    if( Type::Text == type )
        return builder.getInt8PtrTy();

    return builder.getVoidTy();
}
} // namespace llvm
