#include "compiler.hxx"
#include "ast.hxx"
#include "parser.hxx"
#include "aslisp.hxx"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <format>
#include <fstream>
#include <iostream>
#include <memory>

namespace basic {

bool compile(const std::filesystem::path& source, bool generateIr, bool generateLisp)
{
    // ստուգել ֆայլի գոյությունը
    if( !std::filesystem::exists(source) ) {
        std::cerr << "Ֆայլը չի գտնվել։";
        return false;
    }

    // վերլուծություն
    auto file = std::ifstream{source};
    Scanner scanner{file};
    Diagnostics diagnostics;
    Parser parser{scanner, diagnostics};
    auto program = parser.parse();

    // սխալների առկայության դեպքում ծառը թերի է, ուստի հաջորդ փուլերին
    // չի փոխանցվում
    if( !diagnostics.errors().empty() ) {
        for( const auto& error : diagnostics.errors() )
            std::cerr << source.string() << ":" << error << std::endl;

        if( diagnostics.count() > diagnostics.errors().size() )
            std::cerr << std::format("... և ևս {} սխալ։",
                    diagnostics.count() - diagnostics.errors().size()) << std::endl;

        return false;
    }

    // AST-ի գեներացիա Lisp տեսքով
    if( generateLisp ) {
        auto lispPath = source;
        lispPath.replace_extension("lisp");
        std::ofstream ofs{lispPath};
        Lisper().emit(program, ofs);
        return true;
    }

    const std::filesystem::path selfPath = 
            llvm::sys::fs::getMainExecutable(nullptr, nullptr);
    const auto libraryPath = selfPath.parent_path() / "basic_ir_lib.ll";

    llvm::LLVMContext context;

    // կարդալ գրադարանի մոդուլը
    llvm::SMDiagnostic d1;
    auto libraryModule = llvm::parseAssemblyFile(libraryPath.string(), d1, context);

    /*
    // կառուցել ծրագրի LLVM մոդուլիը
    auto programModule = std::make_unique<llvm::Module>(source.string(), context);
    if( !IrEmitter(context, *programModule.get()).emitFor(program) )
        return false;

    // // ստեղծել առանձին ֆայլ
    // if( generateIr ) {
    //     auto irModule = source;
    //     irModule.replace_extension("bas.ll");
    // }

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
    */
    return true;
}

} // namespace basic

