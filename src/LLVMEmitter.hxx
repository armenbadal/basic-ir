
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

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
    void processIf(If* ifSt);
    void processStatement(Statement* stat);
    llvm::BasicBlock* processSequence(Sequence* seq, llvm::Function* parent = nullptr, const std::string& name = "");
    llvm::Value* processExpression(Expression* expr);
    llvm::Value* processBinary(Binary* bin);
    llvm::Value* processUnary(Unary* un);
    llvm::Constant* emitConstant(Number* num);

    llvm::AllocaInst* emitAlloca(Variable* var);
    llvm::LoadInst* emitLoad(Variable* var);

    llvm::Value* getEmittedNode(AstNode* node);
    llvm::Value* getVariableAddress(Variable* var);

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
    std::unordered_map<AstNode*, llvm::Value*> mAddresses;

public:
    static llvm::LLVMContext llvmContext;
};

} // namespace basic
