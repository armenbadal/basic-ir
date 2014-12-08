
#include <llvm/Linker/Linker.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/SourceMgr.h>

#include <llvm/Support/raw_os_ostream.h>

#include "parser.hxx"
#include "compiler.hxx"
#include "support.hxx"

/**/
void compile(const std::string& name, bool lisp)
{
  // վերլուծություն
  Parser sc{name};
  auto moduleAst = sc.parse();

  // արտածման ֆայլերի անուններ
  std::string basen{name};
  auto p = basen.find_last_of(".");
  if( p != std::string::npos ) basen.erase(p);
  std::string irout{basen + ".ll"};
  
  // LLVM IR կոդի գեներացիա
  llvm::IRBuilder<> builder{llvm::getGlobalContext()};
  moduleAst->code(builder);

  // արտածել Լիսպ կոդը
  if( lisp ) {
    std::ofstream slisp{basen + ".lisp"};
    moduleAst->lisp(slisp);
    slisp.close();
  }

  llvm::SMDiagnostic er;
  llvm::Linker link{moduleAst->getCompiled()};
  auto suplib = llvm::ParseAssemblyString(basic_ir_support_library, nullptr, er, llvm::getGlobalContext());
  link.linkInModule(suplib, nullptr);

  std::ofstream sout{irout};
  llvm::raw_os_ostream roo{sout};
  llvm::PrintModulePass pmp{roo};
  llvm::ModulePassManager pm;
  pm.addPass(pmp);
  pm.run(link.getModule());
  sout.close();

  delete moduleAst;
}



