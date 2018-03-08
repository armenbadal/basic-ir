
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
    IrEmitter(llvm::raw_fd_ostream& out)
        : context(), builder(context), outstream(out)
    {}
    ~IrEmitter()
    {
        //delete module;
    }

    bool emitIrCode( Program* prog );

private:
    void emitProgram( Program* prog );
    void emitSubroutine( Subroutine* subr );

    void emitSequence( Sequence* seq );
    void emitLet( Let* let );
    void emitInput( Input* inp );
    void emitPrint( Print* pri );
    /*
    void emitIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    void emitWhile(While* whileSt, llvm::BasicBlock* endBB);
    */
    void emitFor( For* sfor );
    /*
    void emitCall(Call* cal);
    */

    llvm::Value* emitExpression( Expression* expr );
    llvm::Value* emitBinary( Binary* bin );
    llvm::Value* emitUnary( Unary* una );
    llvm::Value* emitText( Text* txt );
    llvm::Constant* emitNumber( Number* num );
    llvm::LoadInst* emitLoad( Variable* var );

    llvm::Type* llvmType( Type type );

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;

    llvm::Module* module = nullptr;

    llvm::raw_fd_ostream& outstream;

    //std::unordered_map<AstNode*, llvm::Value*> mEmittedNodes;
    std::unordered_map<std::string,llvm::Value*> globaltexts;
    std::unordered_map<std::string,llvm::Value*> varaddresses;
};
} // namespace basic
