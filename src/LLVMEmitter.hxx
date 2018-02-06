
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "llvm/Support/raw_ostream.h"

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
    void processStatement(Statement* stat, llvm::BasicBlock* endBB = nullptr);
    void processSequence(Sequence* seq, llvm::BasicBlock* endBB);
    void processLet(Let* letSt);
    void processIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    void processWhile(While* whileSt, llvm::BasicBlock* endBB);
    void processFor(For* forSt, llvm::BasicBlock* endBB);

    llvm::Value* processExpression(Expression* expr);
    llvm::Value* processBinary(Binary* bin);
    llvm::Value* processUnary(Unary* un);

    llvm::Constant* emitConstant(Number* num);
    llvm::LoadInst* emitLoad(Variable* var);

    llvm::Value* getEmittedNode(AstNode* node);
    llvm::Value* getVariableAddress(const std::string& name);
    llvm::Type* getLLVMType(Type type);

///@name 
public:
    LLVMEmitter(AstNode* node, llvm::raw_fd_ostream& out) 
        : mAst(node)
        , mOut(out)
        , mBuilder(llvmContext) 
    {}
    ~LLVMEmitter() { delete mModule; }

///@name private members
private:
    AstNode* mAst = nullptr;    
    //std::ostringstream& mOut;
    llvm::raw_fd_ostream& mOut;

    llvm::IRBuilder<> mBuilder;
    llvm::Module* mModule = nullptr;
    std::unordered_map<AstNode*, llvm::Value*> mEmittedNodes;
    std::unordered_map<std::string, llvm::Value*> mAddresses;

public:
    static llvm::LLVMContext llvmContext;
};

} // namespace basic
