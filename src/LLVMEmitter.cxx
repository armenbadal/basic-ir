
#include "LLVMEmitter.hxx"
#include "ast.hxx"

#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>

namespace basic {

llvm::LLVMContext LLVMEmitter::llvmContext;

llvm::Value* LLVMEmitter::getEmittedNode(AstNode* node)
{
    if (!node) {
        return nullptr;
    }

    auto i = mEmittedNodes.find(node);
    if (i != mEmittedNodes.end()) {
        return i->second;
    }
    return nullptr;
}

llvm::Type* LLVMEmitter::getLLVMType(Type type)
{
    if (type == Type::Void) {
        return mBuilder.getVoidTy();
    }

    if (type == Type::Number) {
        return mBuilder.getDoubleTy();
    }

    if (type == Type::Text) {
        return mBuilder.getInt8PtrTy();
    }

    assert(!"Undefined type");

    return nullptr;
}

///
void LLVMEmitter::emitModule(Program* prog)
{
    if (!prog) {
        return;
    }
    mModule = new llvm::Module("llvm-ir.ll", llvmContext);
    for (Subroutine* si : prog->members) {
        emitFunction(si);
    }
    mModule->print(llvm::errs(), nullptr);
    mModule->print(mOut, nullptr);
}

void LLVMEmitter::emitFunction(Subroutine* sub)
{
    if (!sub) {
        return;
    }
    auto paramNum = sub->parameters.size();
    std::vector<llvm::Type*> paramTypes;
    paramTypes.insert(paramTypes.begin(), paramNum, mBuilder.getDoubleTy());

    auto retType = getLLVMType(sub->rettype);

    auto ft = llvm::FunctionType::get(retType, paramTypes, false);
    auto fun = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, sub->name, mModule);

    auto bb = llvm::BasicBlock::Create(llvmContext, "entry", fun);
    mBuilder.SetInsertPoint(bb);
    for (auto& arg : fun->args()) {
        auto i = arg.getArgNo();
        auto name = sub->parameters[i];
        arg.setName(name);
        auto addr = mBuilder.CreateAlloca(mBuilder.getDoubleTy(), nullptr, name + "_addr");
        mBuilder.CreateStore(&arg, addr);
        mAddresses.insert({ name, addr });
    }

    // Return value allocation
    llvm::Value* retAddr = nullptr;
    if (!retType->isVoidTy()) {
        retAddr = mBuilder.CreateAlloca(mBuilder.getDoubleTy(), nullptr, "ret_addr");
        mAddresses.insert({ sub->name, retAddr });
    }

    //Last block
    auto endBB = llvm::BasicBlock::Create(llvmContext, "end", fun);

    //Handle the function body
    mBuilder.SetInsertPoint(bb);
    processStatement(sub->body, endBB);

    //Set return statement
    mBuilder.SetInsertPoint(endBB);
    if (retAddr != nullptr) {
        auto ret = mBuilder.CreateLoad(mBuilder.getDoubleTy(), retAddr, "ret_val");
        mBuilder.CreateRet(ret);
    }
    else {
        mBuilder.CreateRetVoid();
    }

    mEmittedNodes.insert({ sub, fun });
}

void LLVMEmitter::processSequence(Sequence* seq, llvm::BasicBlock* endBB)
{
    if (!seq) {
        assert(0);
        return;
    }

    for (auto st : seq->items) {
        processStatement(st, endBB);
    }
}

void LLVMEmitter::processStatement(Statement* stat, llvm::BasicBlock* endBB)
{
    if (!stat) {
        assert(0);
        return;
    }
    if (getEmittedNode(stat)) {
        return;
    }

    stat->printKind();
    switch (stat->kind) {
        case NodeKind::Apply:
            break;
        case NodeKind::Sequence:
            processSequence(static_cast<Sequence*>(stat), endBB);
            break;
        case NodeKind::Input:
            break;
        case NodeKind::Print:
            break;
        case NodeKind::Let:
            processLet(static_cast<Let*>(stat));
            break;
        case NodeKind::If:
            processIf(static_cast<If*>(stat), endBB);
            break;
        case NodeKind::While:
            processWhile(static_cast<While*>(stat), endBB);
            break;
        case NodeKind::For:
            processFor(static_cast<For*>(stat), endBB);
            break;
        case NodeKind::Call:
            break;
        default:
            break;
    }
}

void LLVMEmitter::processLet(Let* letSt)
{
    if (!letSt) {
        assert(0);
        return;
    }
    auto addr = getVariableAddress(letSt->varptr->name);
    assert(addr && "Unallocated variable");

    auto val = processExpression(letSt->expr);
    auto st = mBuilder.CreateStore(val, addr);
}

void LLVMEmitter::processIf(If* ifSt, llvm::BasicBlock* endBB)
{
    if (!ifSt) {
        assert(0);
        return;
    }

    if (getEmittedNode(ifSt)) {
        return;
    }
    auto insertBB = mBuilder.GetInsertBlock();
    auto fun = insertBB->getParent();
    auto cnd = processExpression(ifSt->condition);

    llvm::BasicBlock* decBB = llvm::BasicBlock::Create(llvmContext, "bb", fun, endBB);
    mBuilder.SetInsertPoint(decBB);
    processStatement(ifSt->decision, endBB);

    llvm::BasicBlock* altBB = endBB;
    if (ifSt->alternative) {
        altBB = llvm::BasicBlock::Create(llvmContext, "bb", fun, endBB);
        mBuilder.SetInsertPoint(altBB);
        processStatement(ifSt->alternative, endBB);
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

void LLVMEmitter::processWhile(While* whileSt, llvm::BasicBlock* endBB)
{
    if (!whileSt) {
        assert(0);
        return;
    }

    llvm::BasicBlock* head = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);

    mBuilder.CreateBr(head);

    mBuilder.SetInsertPoint(head);
    auto cnd = processExpression(whileSt->condition);
    auto br = mBuilder.CreateCondBr(cnd, body, endBB);

    mBuilder.SetInsertPoint(body);
    processStatement(whileSt->body, endBB);

    if (!body->getTerminator()) {
        mBuilder.SetInsertPoint(body);
        mBuilder.CreateBr(head);
    }
    mBuilder.SetInsertPoint(endBB);
}

void LLVMEmitter::processFor(For* forSt, llvm::BasicBlock* endBB)
{
    if (!forSt) {
        assert(0);
        return;
    }

    llvm::BasicBlock* head = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);
    llvm::BasicBlock* exit = llvm::BasicBlock::Create(llvmContext, "bb", endBB->getParent(), endBB);

    auto param_addr = getVariableAddress(forSt->parameter->name);
    auto begin = processExpression(forSt->begin);
    mBuilder.CreateStore(begin, param_addr);

    //Setting step 1 by default
    llvm::Value* step = mBuilder.getInt32(1);

    //Looking if step is given
    if (forSt->step) {
        step = processExpression(forSt->step);
    }

    auto end = processExpression(forSt->end);
    mBuilder.CreateBr(head);

    mBuilder.SetInsertPoint(head);
    auto param = mBuilder.CreateLoad(param_addr);
    auto cnd = mBuilder.CreateFCmpOLE(param, end, "le");
    mBuilder.CreateCondBr(cnd, body, endBB);

    //Handling the body
    mBuilder.SetInsertPoint(body);
    processStatement(forSt->body, exit);

    //Incrementing the index
    auto inc_param = mBuilder.CreateFAdd(param, step);
    mBuilder.CreateStore(inc_param, param_addr);

    if (!body->getTerminator()) {
        mBuilder.SetInsertPoint(body);
        mBuilder.CreateBr(head);
    }

    mBuilder.SetInsertPoint(endBB);
}

llvm::Value* LLVMEmitter::processExpression(Expression* expr)
{
    if (!expr) {
        assert(0);
        return nullptr;
    }

    if (auto r = getEmittedNode(expr)) {
        return r;
    }
    if (auto num = dynamic_cast<Number*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return emitConstant(num);
    }
    else if (auto text = dynamic_cast<Text*>(expr)) {
        std::cout << __LINE__ << std::endl;
        //return emitString(num);
    }
    else if (auto var = dynamic_cast<Variable*>(expr)) {
        //std::cout << __LINE__ << "  VAR NAME:" << var->name << std::endl;
        return emitLoad(var);
    }
    else if (auto unary = dynamic_cast<Unary*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return processUnary(unary);
    }
    else if (auto binary = dynamic_cast<Binary*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return processBinary(binary);
    }
    else if (auto apply = dynamic_cast<Apply*>(expr)) {
        std::cout << __LINE__ << std::endl;
        // return emitCall(apply);
    }
    else {
        assert(!"Invalid expression");
    }
    return nullptr;
}

llvm::Value* LLVMEmitter::getVariableAddress(const std::string& name)
{
    auto it = mAddresses.find(name);
    if (it != mAddresses.end()) {
        return it->second;
    }
    auto alloca = mBuilder.CreateAlloca(mBuilder.getDoubleTy(), nullptr, name + "_addr");
    mAddresses.insert({ name, alloca });
    return alloca;
}

llvm::LoadInst* LLVMEmitter::emitLoad(Variable* var)
{
    if (!var) {
        return nullptr;
    }

    auto addr = getVariableAddress(var->name);
    llvm::LoadInst* load = nullptr;
    if (var->type == Type::Text) {
        //TODO
    }
    else {
        load = mBuilder.CreateLoad(mBuilder.getDoubleTy(), addr, var->name);
    }
    llvm::errs() << *load << "\n";

    return load;
}

llvm::Value* LLVMEmitter::processBinary(Binary* bin)
{
    if (!bin) {
        assert(0);
        return nullptr;
    }

    if (auto r = getEmittedNode(bin)) {
        return r;
    }
    llvm::Value* lhs = processExpression(bin->subexpro);
    assert(lhs);
    llvm::Value* rhs = processExpression(bin->subexpri);
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

llvm::Value* LLVMEmitter::processUnary(Unary* un)
{
    if (!un) {
        assert(0);
        return nullptr;
    }
    llvm::Value* val = processExpression(un->subexpr);
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

llvm::Constant* LLVMEmitter::emitConstant(Number* num)
{
    if (!num) {
        assert(0);
        return nullptr;
    }

    return llvm::ConstantFP::get(mBuilder.getDoubleTy(), num->value);
}

} // namespace llvm
