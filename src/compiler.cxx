
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
bool compile(const std::filesystem::path& source, bool generaeIr, bool generateLisp)
{
std::clog << source.string() << std::endl;
    // ստուգել ֆայլի գոյությունը
    if( !std::filesystem::exists(source) )
        return false;
    
    // վերլուծություն
    ProgramPtr prog = Parser(source).parse();
    if( nullptr == prog )
        return false;

    // տիպերի ստուգում
    if( const auto ce = Checker().check(prog); ce.has_value() ) {
        std::cerr <<  "TODO: print error message\n";
        std::cerr << ce.value() << std::endl;
        return false;
    }

    // IR կոդի գեներացիա
    if( generaeIr ) {
        auto irModule = source;
        irModule.replace_extension("ll");

        if( !IrEmitter().emitIr(prog, irModule) )
            return false;

        // TODO: սա տեղափոխե՞լ առանձին ֆայլ
        // TODO: վերակազմակերպել ֆայլերի անունները
        // կապակցում գրադարանի հետ
        llvm::LLVMContext context;

        // կարդալ մեր մոդուլը
        llvm::SMDiagnostic d0;
        auto mpro = llvm::parseAssemblyFile(irModule.string(), d0, context);
		if( d0.getSourceMgr() != nullptr ) {
		    llvm::errs() << d0.getMessage() << '\n' << d0.getLineContents() << '\n';
		    return false;
		}

        // կարդալ գրադարանի մոդուլը
        llvm::SMDiagnostic d1;
        const std::filesystem::path selfPath = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
        const auto libraryPath = selfPath.parent_path() / "basic_ir_lib.ll";
        auto mlib = llvm::parseAssemblyFile(libraryPath.string(), d1, context);

        // ստեղծել փուչ մոդուլ
        auto irModuleAll = source;
        irModuleAll.replace_extension("all.ll");
        auto mall = std::make_unique<llvm::Module>(irModuleAll.string(), context);
        
        // կիրառել Linker::linkModules ստատիկ մեթոդը
        llvm::Linker::linkModules(*mall, std::move(mpro));
        llvm::Linker::linkModules(*mall, std::move(mlib));
        
        // ստուգել վերջնական արդյունքը
        llvm::verifyModule(*mall);
        
        // կապակցված մոդուլը գրել ֆայլում
        std::error_code erco;
        llvm::raw_fd_ostream _fout(irModuleAll.string(), erco, llvm::sys::fs::OF_None);

        llvm::legacy::PassManager passer;
        passer.add(llvm::createPrintModulePass(_fout, ""));
        passer.run(*mall);
        
        ///* DEBUG */ mall->print(llvm::errs(), nullptr);
    }

    // AST-ի գեներացիա Lisp տեսքով
    if( generateLisp ) {
        auto lispPath = source;
        lispPath.replace_extension("lisp");
        if( !Lisper().emitLisp(prog, lispPath) )
            return false;
    }
      
    return true;
}

} // basic

