
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
    if (!node) { return nullptr; }

    auto i = mEmittedNodes.find(node);
    if (i != mEmittedNodes.end()) {
        return i->second;
    }
    return nullptr;
}

///
void LLVMEmitter::emitModule(Program* prog)
{
    if (! prog) { return; }
    mModule = new llvm::Module("llvm-ir.ll", llvmContext);
    //mOut << "(basic-program :filename " << prog->filename;
    //mOut << " :members (";
    for( Subroutine* si : prog->members ) {
        //mOut << "Name: " << (*si).name <<  std::endl;
        emitFunction(si);
    }
    mModule->print(llvm::errs(), nullptr);
}

void LLVMEmitter::emitFunction(Subroutine* sub)
{
    if (! sub) { return; }
    auto paramNum = sub->parameters.size();
    std::vector<llvm::Type*> paramTypes;
    paramTypes.insert(paramTypes.begin(), paramNum, mBuilder.getInt32Ty());

    auto ft = llvm::FunctionType::get(mBuilder.getVoidTy(), paramTypes, false);
    auto fun = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, sub->name, mModule);
    //auto bb = llvm::BasicBlock::Create(llvmContext, "entry", fun);
    auto body = static_cast<Sequence*>(sub->body);
    processSequence(body, fun, "entry");
    
    for (auto& arg : fun->args()) {
        auto i = arg.getArgNo();
        auto name = sub->parameters[i];
        arg.setName(name);
        auto addr = mBuilder.CreateAlloca(mBuilder.getDoubleTy(), nullptr, name + "_addr");
        mBuilder.CreateStore(&arg, addr);
        mAddresses.insert({&arg, addr});
    }

    mEmittedNodes.insert({sub, fun});
}

llvm::BasicBlock* LLVMEmitter::processSequence(Sequence* seq, llvm::Function* parent, const std::string& name)
{
    if (! seq) { assert(0);return nullptr; }

    if (auto ret = getEmittedNode(seq)) { return llvm::cast<llvm::BasicBlock>(ret); }
    auto bb = llvm::BasicBlock::Create(llvmContext, name, parent);
    mBuilder.SetInsertPoint(bb);
    for (auto st : seq->items) {
        processStatement(st);
    }
    //FIXME terminator misssing
    mEmittedNodes.insert({seq, bb});
    return bb;
}

void LLVMEmitter::processStatement(Statement* stat)
{
    if (! stat) { assert(0);return; }
     
    if (getEmittedNode(stat)) { return ; }
    stat->printKind();
    switch (stat->kind) {
        case NodeKind::Number:
            break;
        case NodeKind::Text:
            break;
        case NodeKind::Variable:
            break;
        case NodeKind::Unary:
            break;
        case NodeKind::Binary:
            break;
        case NodeKind::Apply:
            break;
        case NodeKind::Sequence:
            break;
        case NodeKind::Input:
            break;
        case NodeKind::Print:
            break;
        case NodeKind::Let:
            break;
        case NodeKind::If:
            processIf(static_cast<If*>(stat));
            break;
        case NodeKind::While:
            break;
        case NodeKind::For:
            break;
        case NodeKind::Call:
            break;
        default : 
            break;
    }
}

void LLVMEmitter::processIf(If* ifSt)
{
    if (! ifSt) { assert(0);return; }

    if (getEmittedNode(ifSt)) { return ; }
    auto insertBB = mBuilder.GetInsertBlock();
    auto fun = insertBB->getParent();
    auto cnd = processExpression(ifSt->condition);
    llvm::BasicBlock* decBB = processSequence(static_cast<Sequence*>(ifSt->decision), fun);
    llvm::BasicBlock* altBB = processSequence(static_cast<Sequence*>(ifSt->alternative), fun);

    mBuilder.SetInsertPoint(insertBB);
    auto br = mBuilder.CreateCondBr(cnd, decBB, altBB);

    mEmittedNodes.insert({ifSt, br});
}

llvm::Value* LLVMEmitter::processExpression(Expression* expr)
{
    if (! expr) {assert(0); return nullptr; }

    if (auto r = getEmittedNode(expr)) { return r; }
    if (auto num = dynamic_cast<Number*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return emitConstant(num);
    } else if (auto text = dynamic_cast<Text*>(expr)) {
        std::cout << __LINE__ << std::endl;
        //return emitString(num);
    } else if (auto var = dynamic_cast<Variable*>(expr)) {
        std::cout << __LINE__ << "  VAR NAME:" << var->name << std::endl;
        //return emitAlloca(var);
        return emitLoad(var);
    } else if (auto unary = dynamic_cast<Unary*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return processUnary(unary);
    } else if (auto binary = dynamic_cast<Binary*>(expr)) {
        std::cout << __LINE__ << std::endl;
        return processBinary(binary);
    } else if (auto apply = dynamic_cast<Apply*>(expr)) {
        std::cout << __LINE__ << std::endl;
       // return emitCall(apply);
    } else {
        assert(!"Invalid expression");  
    }
    return nullptr;
}

llvm::Value* LLVMEmitter::getVariableAddress(Variable* var)
{
    if (!var) { return nullptr; }
    auto it = mAddresses.find(var);
    if (it != mAddresses.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::LoadInst* LLVMEmitter::emitLoad(Variable* var)
{
    if (!var) { return nullptr; }

    auto addr = getVariableAddress(var);
    llvm::LoadInst* load = nullptr;
    if (var->type == Type::Text) {
        //TODO
    } else {
        load = mBuilder.CreateLoad(mBuilder.getDoubleTy(), addr, var->name);
    }
    llvm::errs() << *load << "\n";
    
    return load;
}

llvm::AllocaInst* LLVMEmitter::emitAlloca(Variable* var)
{
    if (!var) { return nullptr; }

    llvm::AllocaInst* alloca = nullptr;
    if (var->type == Type::Text) {
        //TODO
    } else {
        alloca = mBuilder.CreateAlloca(mBuilder.getDoubleTy(), nullptr, var->name + "_addr");
        llvm::errs() << *alloca << "\n";
        mAddresses.insert({var, alloca});
    }
    
    return alloca;
}

llvm::Value* LLVMEmitter::processBinary(Binary* bin)
{
    if (! bin)  { assert(0); return nullptr; }

    if (auto r = getEmittedNode(bin)) { return r; }
    std::cout << "Processing binary ... " << __LINE__ << std::endl;
    llvm::Value* lhs = processExpression(bin->subexpro); assert(lhs);
    llvm::Value* rhs = processExpression(bin->subexpri); assert(rhs);
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
            //ret = mBuilder.CreateFCmpOGT(lhs, rhs, "ne");
            ret = mBuilder.CreateICmpSGT(lhs, rhs, "gt");
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
            assert("CONC operator is not handled yet");
            break;
        default: {
            assert("Undefined binary operator");
            break;
        }
    }
    mEmittedNodes.insert({bin, ret});
    return ret;
}

llvm::Value* LLVMEmitter::processUnary(Unary* un)
{
    if (! un)  { assert(0);return nullptr; }
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
    if (! num) { assert(0);return nullptr; }
    
    return llvm::ConstantFP::get(mBuilder.getDoubleTy(), num->value);
}



} // namespace llvm
