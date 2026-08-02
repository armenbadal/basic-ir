#pragma once

#include "ast.hxx"
#include "astvisitor.hxx"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace llvm {
class BasicBlock;
class CallInst;
class Constant;
class Function;
class Module;
class Type;
class UnaryInstruction;
class Value;
} // namespace llvm

namespace basic {

class IrEmitter : public ASTVisitorBase {
public:
    IrEmitter(llvm::LLVMContext& cx, llvm::Module& md);

    bool emitFor(Program::Ptr prog);

private:
    using ASTVisitorBase::visit;

    void visit(Program::Ptr prog) override;
    void visit(Subroutine::Ptr subr) override;

    void visit(Sequence::Ptr seq) override;
    void visit(Dim::Ptr dim) override;
    void visit(Let::Ptr let) override;
    void visit(If::Ptr sif) override;
    void visit(For::Ptr sfor) override;
    void visit(While::Ptr swhi) override;
    void visit(Call::Ptr cal) override;

    void visit(Array::Ptr arr) override;
    void visit(Apply::Ptr apy) override;
    void visit(Binary::Ptr bin) override;
    void visit(Unary::Ptr una) override;
    void visit(Text::Ptr txt) override;
    void visit(Number::Ptr num) override;
    void visit(Boolean::Ptr num) override;
    void visit(Variable::Ptr var) override;

    llvm::Type* llvmType(std::string_view name);

    void setCurrentBlock(llvm::Function* fun, llvm::BasicBlock* bl);

    void prepareLibrary();
    void declareLibraryFunction(std::string_view name, std::string_view signature);
    llvm::FunctionCallee libraryFunction(std::string_view name);
    llvm::FunctionCallee userFunction(std::string_view name);

    void createEntryPoint();
    void declareSubroutines(Program::Ptr prog);
    void defineSubroutines(Program::Ptr prog);
    bool createsTempText(Expression::Ptr expr);
    llvm::CallInst* createLibraryFuncCall(std::string_view fname,
            const llvm::ArrayRef<llvm::Value*>& args);

private:
    llvm::LLVMContext& context;
    llvm::Module& moduler;
    llvm::IRBuilder<> builder;

    llvm::Value* _result = nullptr;

    std::unordered_map<std::string,llvm::FunctionType*> library;
    std::unordered_map<std::string,llvm::Value*> globalTexts;
    std::unordered_map<std::string,llvm::Value*> varAddresses;

    llvm::Type* VoidTy = builder.getVoidTy();
    llvm::Type* BooleanTy = builder.getInt1Ty();
    llvm::Type* NumericTy = builder.getDoubleTy();
    llvm::Type* TextualTy = builder.getPtrTy();
};

} // namespace basic
