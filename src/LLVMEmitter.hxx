
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>

namespace basic {

/// 
class LLVMEmitter
{

///@name Public interface    
public:
    void emitModule (Program* prog);
    void emitFunction(Subroutine* sub);

///@name internal functions
private:
    void processLet(Let* letSt);
    void processIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    llvm::BasicBlock* processSequence(Sequence* seq, llvm::BasicBlock* bb, llvm::BasicBlock* endBB = nullptr);
    void processStatement(Statement* stat, llvm::BasicBlock* endBB = nullptr);
    llvm::Value* processExpression(Expression* expr);
    llvm::Value* processBinary(Binary* bin);
    llvm::Value* processUnary(Unary* un);
    llvm::Constant* emitConstant(Number* num);

    llvm::AllocaInst* emitAlloca(Variable* var);
    llvm::LoadInst* emitLoad(Variable* var);

    llvm::Value* getEmittedNode(AstNode* node);
    llvm::Value* getVariableAddress(const std::string& name);

///@name 
public:
    LLVMEmitter(AstNode* node, std::ostringstream& out) 
        : mAst(node)
        , mOut(out)
        , mBuilder(llvmContext) 
    {}
    ~LLVMEmitter() { delete mModule; }

///@name private members
private:
    AstNode* mAst = nullptr;    
    std::ostringstream& mOut;

    llvm::IRBuilder<> mBuilder;
    llvm::Module* mModule = nullptr;
    std::unordered_map<AstNode*, llvm::Value*> mEmittedNodes;
    std::unordered_map<std::string, llvm::Value*> mAddresses;

public:
    static llvm::LLVMContext llvmContext;
};

} // namespace basic
