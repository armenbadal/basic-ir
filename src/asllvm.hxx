
#ifndef ASLLVM_HXX
#define ASLLVM_HXX

#include "ast.hxx"
#include "converter.hxx"

#include <ostream>


class IrEmitter : public Converter {

private:
    int convertProgram(Program* node) override;
    int convertSubroutine(Subroutine* node) override;

    int convertSequence(Sequence* node) override;
    int convertLet(Let* node) override;
    int convertInput(Input* node) override;
    int convertPrint(Print* node) override;
    int convertIf(If* node) override;
    int convertWhile(While* node) override;
    int convertFor(For* node) override;
    int convertCall(Call* node) override;

    int convertApply(Apply* node) override;
    int convertBinary(Binary* node) override;
    int convertUnary(Unary* node) override;
    int convertVariable(Variable* node) override;
    int convertText(Text* node) override;
    int convertNumber(Number* node) override;

/// Helper functions
private:

    void processStatement(Statement* stat, llvm::BasicBlock* endBB = nullptr);
    void processSequence(Sequence* seq, llvm::BasicBlock* endBB);
    void processIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    void processWhile(While* whileSt, llvm::BasicBlock* endBB);
    void processFor(For* forSt, llvm::BasicBlock* endBB);


private:
    int indent = 0;

    void space(std::ostream& ooo);
    
///@name private members
private:
    AstNode* mAst = nullptr;
    //std::ostringstream& mOut;
    llvm::raw_fd_ostream& mOut;

    llvm::IRBuilder<> mBuilder;
    llvm::Module* mModule = nullptr;
    std::unordered_map<AstNode*, llvm::Value*> mEmittedNodes;
    std::unordered_map<std::string, llvm::Value*> mAddresses;
};

#endif // ASLLVM_HXX

