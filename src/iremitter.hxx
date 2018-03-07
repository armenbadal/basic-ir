
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>

namespace basic {

///
class IrEmitter {
public:
    IrEmitter(AstNode* node, llvm::raw_fd_ostream& out)
        : mAst(node), mOut(out), mBuilder(llvmContext)
    {}
    ~IrEmitter() { delete module; }

private:
    void emitProgram(Program* prog);
    void emitSubroutine(Subroutine* sub);

    void emitStatement(Statement* stat, llvm::BasicBlock* endBB = nullptr);

    void emitSequence(Sequence* seq, llvm::BasicBlock* endBB);
    void emitLet(Let* let);
    void emitInput(Input* inp);
    void emitPrint(Print* pri);
    void emitIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    void emitWhile(While* whileSt, llvm::BasicBlock* endBB);
    void emitFor(For* forSt, llvm::BasicBlock* endBB);
    void emitCall(Call* cal);

    llvm::Value* emitExpression(Expression* expr);
    llvm::Value* emitBinary(Binary* bin);
    llvm::Value* emitUnary(Unary* un);

    llvm::Constant* emitConstant(Number* num);
    llvm::LoadInst* emitLoad(Variable* var);

    llvm::Value* getEmittedNode(AstNode* node);
    llvm::Value* getVariableAddress(const std::string& name);
    llvm::Type* getLLVMType(Type type);

private:
    AstNode* mAst = nullptr;
    //std::ostringstream& mOut;
    llvm::raw_fd_ostream& mOut;

    llvm::IRBuilder<> mBuilder;
    llvm::Module* module = nullptr;
    std::unordered_map<AstNode*, llvm::Value*> mEmittedNodes;
    std::unordered_map<std::string, llvm::Value*> mAddresses;

    static llvm::LLVMContext llvmContext;
};
} // namespace basic
