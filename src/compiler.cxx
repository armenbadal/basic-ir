
#include "compiler.hxx"
#include "ast.hxx"
#include "parser.hxx"
#include "typechecker.hxx"
#include "iremitter.hxx"
#include "aslisp.hxx"

#include <fstream>
#include <iostream>

namespace basic {

///
bool fileExists(const std::string& filename)
{
    std::ifstream infi(filename);
    return infi.good();
}

///
bool compile( const std::string& bas, bool ir, bool lisp )
{
    // ստուգել ֆայլի գոյությունը
    bool filex = fileExists(bas);
    if( !filex )
        return false;
    
    // վերլուծություն
    ProgramPtr prog = Parser(bas).parse();
    if( nullptr == prog )
        return false;

    // տիպերի ստուգում
    bool errok = TypeChecker().check(prog);
    if( !errok )
        return false;

    // IR կոդի գեներացիա
    if( ir ) {
        bool irok = IrEmitter(/*ֆայլի անուն*/).emitIrCode(prog);
        if( !irok )
            return false;

        // TODO: կապակցում գրադարանի հետ
        // 1. կարդալ մեր մոդուլը
        // 2. կարդալ գրադարանի մոդուլը
        // 3. ստեղծել փուչ մոդուլ
        // 4. կիրառել Linker::linkModules ստատիկ մեթոդը
        // 5. կապակցված մոդուլը գրել ֆայլում
        
        // bool Linker::linkModules(
        //   Module& Dest,
        //   std::unique_ptr<Module> Src,
        //   unsigned Flags = Flags::None,
        //   std::function<void(Module&, const StringSet<>&)> InternalizeCallback = {}) 

    }

    // AST-ի գեներացիա Lisp տեսքով
    if( lisp ) {
        // TODO: cout-ը փոխարինել ֆայլով
        bool lispok = Lisper(std::cout).asLisp(prog);
        if( !lispok )
            return false;
    }
        
    return true;
}

} // basic

