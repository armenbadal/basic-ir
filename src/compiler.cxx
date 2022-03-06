
#include "compiler.hxx"
#include "ast.hxx"
#include "parser.hxx"
#include "checker.hxx"
#include "iremitter.hxx"
#include "aslisp.hxx"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <fstream>
#include <iostream>
#include <memory>

#include <unistd.h>

namespace basic {

///
bool fileExists( const std::string& filename )
{
    std::ifstream infi(filename);
    return infi.good();
}

///
std::string libraryPath()
{
    // գրադարանի ֆայլը basic-ir-ի կողքին է, դրա ճանապարհը
    // գտնելու համար օգտագործել readlink("/proc/self/exe")

    // TODO: ավելացնել արժեքների ստուգումներ
    
    const char* cname = "basic-ir";
    const size_t psize = 1024;
    char execpath[psize] = { 0 };
    ssize_t rls = readlink("/proc/self/exe", execpath, psize-1);
    execpath[rls-strlen(cname)] = '\0';
    return std::string(execpath) + "/basic_ir_lib.ll";
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
    bool errok = Checker(prog).check(std::cerr);
    if( !errok )
        return false;

    // IR կոդի գեներացիա
    if( ir ) {
        bool irok = IrEmitter(prog).emitIr(bas + ".ll");
        if( !irok )
            return false;

        // TODO: սա տեղափոխե՞լ առանձին ֆայլ
        // TODO: վերակազմակերպել ֆայլերի անունները
        // կապակցում գրադարանի հետ
        llvm::LLVMContext context;
        // կարդալ մեր մոդուլը
        llvm::SMDiagnostic d0;
        auto mpro = llvm::parseAssemblyFile(bas + ".ll", d0, context);
		if( d0.getSourceMgr() != nullptr ) {
		  llvm::errs() << d0.getMessage() << '\n' << d0.getLineContents() << '\n';
		  return false;
		}
        // կարդալ գրադարանի մոդուլը
        llvm::SMDiagnostic d1;
        auto mlib = llvm::parseAssemblyFile(libraryPath(), d1, context);
        // ստեղծել փուչ մոդուլ
        auto mall = std::make_unique<llvm::Module>(bas + "_all.ll", context);
        // կիրառել Linker::linkModules ստատիկ մեթոդը
        llvm::Linker::linkModules(*mall, std::move(mpro));
        llvm::Linker::linkModules(*mall, std::move(mlib));
        // ստուգել վերջնական արդյունքը
        llvm::verifyModule(*mall);
        // կապակցված մոդուլը գրել ֆայլում
        std::error_code erco;
        llvm::raw_fd_ostream _fout(bas + "_all.ll", erco, llvm::sys::fs::OF_None);

        llvm::legacy::PassManager passer;
        passer.add(llvm::createPrintModulePass(_fout, ""));
        passer.run(*mall);
        
        ///* DEBUG */ mall->print(llvm::errs(), nullptr);
    }

    // AST-ի գեներացիա Lisp տեսքով
    if( lisp ) {
        // TODO: cout-ը փոխարինել ֆայլով
        bool lispok = Lisper(prog).emitLisp(bas + ".lisp");
        if( !lispok )
            return false;
    }
        
    return true;
}

} // basic

