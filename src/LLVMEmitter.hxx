
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>

#include <iostream>
#include <sstream>

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
    void processIf(If* ifSt, llvm::BasicBlock* bb);
    void processStatement(Statement* stat, llvm::BasicBlock* bb);
    llvm::BasicBlock* processSequence(Sequence* seq, llvm::Function* parent, const std::strin& name);

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

public:
    static llvm::LLVMContext llvmContext;
};

} // namespace basic
