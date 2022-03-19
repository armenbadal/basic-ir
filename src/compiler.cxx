
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

#include <iostream>
#include <memory>

namespace basic {

///
bool compile(const std::filesystem::path& source, bool generaeIr, bool generateLisp)
{
    const std::filesystem::path selfPath = 
            llvm::sys::fs::getMainExecutable(nullptr, nullptr);
    const auto libraryPath = selfPath.parent_path() / "basic_ir_lib.ll";

    llvm::LLVMContext context;

    // կարդալ գրադարանի մոդուլը
    llvm::SMDiagnostic d1;
    std::unique_ptr<llvm::Module> libraryModule = 
            llvm::parseAssemblyFile(libraryPath.string(), d1, context);

    // const llvm::Module::FunctionListType& functions = libraryModule->getFunctionList();
    // for( const auto& func : functions )
    //     if( !func.isDeclaration() )
    //         std::clog << func.getName().str() << std::endl;


    // ստուգել ֆայլի գոյությունը
    if( !std::filesystem::exists(source) )
        return false;
    
    // վերլուծություն
    ProgramPtr program = Parser(source).parse();
    if( nullptr == program )
        return false;

    // տիպերի ստուգում
    if( const auto ce = Checker().check(program); ce.has_value() ) {
        std::cerr <<  "TODO: print error message\n";
        std::cerr << ce.value() << std::endl;
        return false;
    }

    // IR կոդի գեներացիա
    if( generaeIr ) {
        auto irModule = source;
        irModule.replace_extension("ll");

        if( !IrEmitter().emit(program, irModule) )
            return false;

        // TODO: սա տեղափոխե՞լ առանձին ֆայլ
        // TODO: վերակազմակերպել ֆայլերի անունները
        // կապակցում գրադարանի հետ

        // կարդալ մեր մոդուլը
        llvm::SMDiagnostic d0;
        auto programModule = llvm::parseAssemblyFile(irModule.string(), d0, context);
		if( d0.getSourceMgr() != nullptr ) {
		    llvm::errs() << d0.getMessage() << '\n' << d0.getLineContents() << '\n';
		    return false;
		}

        // ստեղծել փուչ մոդուլ
        auto irModuleAll = source;
        irModuleAll.replace_extension("all.ll");
        auto linkedModule = std::make_unique<llvm::Module>(irModuleAll.string(), context);
        
        // կիրառել Linker::linkModules ստատիկ մեթոդը
        llvm::Linker::linkModules(*linkedModule, std::move(programModule));
        llvm::Linker::linkModules(*linkedModule, std::move(libraryModule));
        
        // կապակցված մոդուլը գրել ֆայլում
        std::error_code ec;
        llvm::raw_fd_ostream _fout(irModuleAll.string(), ec, llvm::sys::fs::OF_None);
        if( !ec )
            return false;

        llvm::legacy::PassManager passer;
        passer.add(llvm::createVerifierPass()); // ստուգել վերջնական արդյունքը
        passer.add(llvm::createPrintModulePass(_fout, ""));
        passer.run(*linkedModule);
        
        ///* DEBUG */ mall->print(llvm::errs(), nullptr);
    }

    // AST-ի գեներացիա Lisp տեսքով
    if( generateLisp ) {
        auto lispPath = source;
        lispPath.replace_extension("lisp");
        if( !Lisper().emitLisp(program, lispPath) )
            return false;
    }
      
    return true;
}

} // namespace basic

