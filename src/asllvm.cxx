
#include "ast.hxx"
#include "LLVMEmitter.hxx"

#include <iostream>
#include <sstream>

namespace basic {

///
void emitLLVM( Program* node, llvm::raw_fd_ostream& ooo )
{
    LLVMEmitter llvm(node, ooo);
    llvm.emitModule(node);
     
    
    
    //ooo << "(basic-program :filename " << node->filename;
    //ooo << " :members (";
    //for( Subroutine* si : node->members ) {
    //    ooo << "Name: " << (*si).name <<  std::endl;
    //    if (auto body = dynamic_cast<Sequence*>((*si).body)) {
    //        for (auto st : body->items) {
    //            st->printKind();
    //        }
    //    }
    //    //llvm(si, ooo);
    //}
    //ooo << "))\n";
}

} // namespace llvm
