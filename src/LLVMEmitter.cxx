
#include "LLVMEmitter.hxx"
#include "ast.hxx"

#include <llvm/IR/GlobalValue.h>

#include <iostream>
#include <sstream>

namespace basic {

llvm::LLVMContext LLVMEmitter::llvmContext;

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

        //if (auto body = dynamic_cast<Sequence*>(si->body)) {
        //    for (auto st : body->items) {
        //        st->printKind();
        //    }
        //}
    }
    mModule->dump();
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
}

llvm::BasicBlock* LLVMEmitter::processSequence(Sequence* seq, llvm::Function* parent, const std::strin& name)
{
    if (! seq || ! parent) { return; }
    auto bb = llvm::BasicBlock::Create(llvmContext, name, parent);
    for (auto st : seq->items) {
        processStatement(st, bb);
    }
    //FIXME terminator misssing
    return bb;
}

void LLVMEmitter::processStatement(Statement* stat, llvm::BasicBlock* bb)
{
    if (! stat || ! bb) { return; }
     
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
            processIf(static_cast<If*>(stat), bb);
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

void LLVMEmitter::processIf(If* ifSt, llvm::BasicBlock* bb)
{
    if (! ifSt || ! bb) { return; }
    llvm::Instruction* cnd = processExpression(ifSt->condition, bb);
    llvm::BasicBlock* decBB = processSequence(static_cast<Sequence*>(ifSt->decision), bb->getParent());
    llvm::BasicBlock* altBB = processSequence(static_cast<Sequence*>(ifSt->alternative), bb->getParent());
    emitCndBr(cnd, decBB, altBB);
}


} // namespace llvm
