
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
    using String = std::string;
    using IrType = llvm::Type;
    using TypeVector = std::vector<IrType*>;

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
    void emitFor( ForPtr sfor );
    //void emitWhile(While* whileSt, llvm::BasicBlock* endBB);
    void emitCall( CallPtr cal );

    llvm::Value* emitExpression( ExpressionPtr expr );
    llvm::Value* emitApply( ApplyPtr apy );
    llvm::Value* emitBinary( BinaryPtr bin );
    llvm::Value* emitUnary( UnaryPtr una );
    llvm::Value* emitText( TextPtr txt );
    llvm::Constant* emitNumber( NumberPtr num );
    llvm::LoadInst* emitLoad( VariablePtr var );

    llvm::Type* llvmType( Type type );
    void declareFunction( const String& name, const TypeVector& patys,
        IrType* rty, bool external = false );
    void declareLibrary();
    void declareSubroutines( ProgramPtr prog );
    void defineSubroutines( ProgramPtr prog );

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;

    std::unique_ptr<llvm::Module> module = nullptr;

    llvm::raw_fd_ostream& outstream;

    std::unordered_map<String,llvm::Value*> globaltexts;
    std::unordered_map<String,llvm::Value*> varaddresses;
};
} // namespace basic
