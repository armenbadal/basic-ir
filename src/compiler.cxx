#include "compiler.hxx"
#include "ast.hxx"
#include "parser.hxx"
#include "checker.hxx"
#include "iremitter.hxx"
#include "aslisp.hxx"

#include <llvm/Pass.h>
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
std::unique_ptr<llvm::Module> compileBasicIR(llvm::LLVMContext& context, const std::filesystem::path& source)
{
    // ստուգել ֆայլի գոյությունը
    if( !std::filesystem::exists(source) ) {
        std::cerr << "Ֆայլը չի գտնվել։";
        return nullptr;
    }

     // վերլուծություն
    ProgramPtr program = Parser(source).parse();
    if( nullptr == program ) {
        std::cerr << "Վերլուծության սխալ։";
        return nullptr;
    }

    // տիպերի ստուգում
    if( const auto ce = Checker().check(program); ce.has_value() ) {
        std::cerr << "TODO: print error message\n";
        std::cerr << ce.value() << std::endl;
        return nullptr;
    }

    // LLVM մոդուլի կառուցում
    auto pm = std::make_unique<llvm::Module>(source.string(), context);
    if( !IrEmitter(context, *pm.get()).emitFor(program) )
        return nullptr;

    return pm;
}

///
bool compile(const std::filesystem::path& source, bool generateIr, bool generateLisp)
{
    const std::filesystem::path selfPath = 
            llvm::sys::fs::getMainExecutable(nullptr, nullptr);
    const auto libraryPath = selfPath.parent_path() / "basic_ir_lib.ll";

    llvm::LLVMContext context;

    // կարդալ գրադարանի մոդուլը
    llvm::SMDiagnostic d1;
    auto libraryModule = llvm::parseAssemblyFile(libraryPath.string(), d1, context);

    // կառուցել ծրագրի մոդուլը
    auto programModule = compileBasicIR(context, source);

//    // ստեղծել առանձին ֆայլ
//    if( generateIr ) {
//        auto irModule = source;
//        irModule.replace_extension("bas.ll");
//    }

    // կապակցել երկու մոդուլները
    // ստեղծել փուչ մոդուլ
    auto irModuleAll = source;
    irModuleAll.replace_extension("ll");
    auto linkedModule = std::make_unique<llvm::Module>(irModuleAll.string(), context);

    // կիրառել Linker::linkModules ստատիկ մեթոդը
    llvm::Linker::linkModules(*linkedModule, std::move(programModule));
    llvm::Linker::linkModules(*linkedModule, std::move(libraryModule));

    // կապակցված մոդուլը գրել ֆայլում
    std::error_code ec;
    llvm::raw_fd_ostream out(irModuleAll.string(), ec, llvm::sys::fs::OF_None);
    if( ec )
        return false;

    llvm::legacy::PassManager pm;
    pm.add(llvm::createVerifierPass()); // ստուգել վերջնական արդյունքը
    pm.add(llvm::createPrintModulePass(out, ""));
    pm.run(*linkedModule.get());

//    // AST-ի գեներացիա Lisp տեսքով
//    if( generateLisp ) {
//        auto lispPath = source;
//        lispPath.replace_extension("lisp");
//        if( !Lisper().emitLisp(program, lispPath) )
//            return false;
//    }
      
    return true;
}

} // namespace basic

