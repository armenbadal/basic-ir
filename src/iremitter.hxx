
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

    bool emitIrCode( std::shared_ptr<Program> prog );

private:
    void emitProgram( std::shared_ptr<Program> prog );
    void emitSubroutine( std::shared_ptr<Subroutine> subr );

    void emitSequence( std::shared_ptr<Sequence> seq );
    void emitLet( std::shared_ptr<Let> let );
    void emitInput( std::shared_ptr<Input> inp );
    void emitPrint( std::shared_ptr<Print> pri );
    /*
    void emitIf(If* ifSt, llvm::BasicBlock* endBB = nullptr);
    void emitWhile(While* whileSt, llvm::BasicBlock* endBB);
    */
    void emitFor( std::shared_ptr<For> sfor );
    /*
    void emitCall(Call* cal);
    */

    llvm::Value* emitExpression( std::shared_ptr<Expression> expr );
    llvm::Value* emitBinary( std::shared_ptr<Binary> bin );
    llvm::Value* emitUnary( std::shared_ptr<Unary> una );
    llvm::Value* emitText( std::shared_ptr<Text> txt );
    llvm::Constant* emitNumber( std::shared_ptr<Number> num );
    llvm::LoadInst* emitLoad( std::shared_ptr<Variable> var );

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
