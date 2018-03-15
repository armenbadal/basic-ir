
#include "ast.hxx"

#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <memory>

namespace basic {

///
class IrEmitter {
public:
    IrEmitter( llvm::raw_fd_ostream& out )
        : context(), builder(context), outstream(out)
    {}
    ~IrEmitter()
    {}

    bool emitIrCode( ProgramPtr prog );

private:
    void emitProgram( ProgramPtr prog );
    void emitSubroutine( SubroutinePtr subr );

    void emitSequence( SequencePtr seq );
    void emitLet( LetPtr let );
    void emitInput( InputPtr inp );
    void emitPrint( PrintPtr pri );

    //void emitIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    //void emitWhile(While* whileSt, llvm::BasicBlock* endBB);

    void emitFor( ForPtr sfor );
    //void emitCall(Call* cal);

    llvm::Value* emitExpression( ExpressionPtr expr );
    llvm::Value* emitBinary( BinaryPtr bin );
    llvm::Value* emitUnary( UnaryPtr una );
    llvm::Value* emitText( TextPtr txt );
    llvm::Constant* emitNumber( NumberPtr num );
    llvm::LoadInst* emitLoad( VariablePtr var );

    void declareLibSubr( const std::string& name, llvm::ArrayRef<llvm::Type*> patys, llvm::Type* rty );
    void declareLibrary();
    llvm::Type* llvmType( Type type );

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;

    std::unique_ptr<llvm::Module> module = nullptr;

    llvm::raw_fd_ostream& outstream;

    std::unordered_map<std::string,llvm::Function*> library;
    std::unordered_map<std::string,llvm::Value*> globaltexts;
    std::unordered_map<std::string,llvm::Value*> varaddresses;
};
} // namespace basic
