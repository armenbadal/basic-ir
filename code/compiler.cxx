
#include <llvm/Linker/Linker.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/SourceMgr.h>

#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/JIT.h>
//#include <llvm/IR/DerivedTypes.h>

#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>

#include "parser.hxx"
#include "compiler.hxx"
#include "support.hxx"

/**/
unsigned int compile(const std::string& name, bool lisp)
{
  // վերլուծություն
  Parser sc{name};
  auto moduleAst = sc.parse();
  if( moduleAst == nullptr ) { 
    delete moduleAst;
    return 1;
  }

  // արտածման ֆայլերի անուններ
  std::string basen{name};
  auto p = basen.find_last_of(".");
  if( p != std::string::npos ) basen.erase(p);
  std::string irout{basen + ".ll"};
  
  // LLVM IR կոդի գեներացիա
  auto& cox = llvm::getGlobalContext();
  llvm::IRBuilder<> builder{cox};
  moduleAst->code(builder);

  // արտածել Լիսպ կոդը
  if( lisp ) {
    std::ofstream slisp{basen + ".lisp"};
    moduleAst->lisp(slisp);
    slisp.close();
  }

  llvm::SMDiagnostic er;
  llvm::Linker link{moduleAst->getCompiled()};
  auto suplib = llvm::ParseAssemblyString(basic_ir_support_library, nullptr, er, cox);
  link.linkInModule(suplib, nullptr);

  std::ofstream sout{irout};
  llvm::raw_os_ostream roo{sout};
  llvm::PrintModulePass pmp{roo};
  llvm::ModulePassManager pm;
  pm.addPass(pmp);
  pm.run(link.getModule());
  sout.close();

  delete moduleAst;
  return 0;
}

/**/
void jitRun(const std::string& name)
{
  // վերլուծություն
  Parser sc{name};
  auto moduleAst = sc.parse();
  if( moduleAst == nullptr ) { 
    delete moduleAst;
    return;
  }

  // արտածման ֆայլերի անուններ
  std::string basen{name};
  auto p = basen.find_last_of(".");
  if( p != std::string::npos ) basen.erase(p);
  std::string irout{basen + ".ll"};
  
  // LLVM IR կոդի գեներացիա
  auto& cox = llvm::getGlobalContext();
  llvm::IRBuilder<> builder{cox};
  moduleAst->code(builder);

  llvm::SMDiagnostic er;
  llvm::Linker link{moduleAst->getCompiled()};
  auto suplib = llvm::ParseAssemblyString(basic_ir_support_library, nullptr, er, cox);
  link.linkInModule(suplib, nullptr);

  auto irmod = moduleAst->getCompiled();
  auto ep = irmod->getFunction("Main");
if( ep == nullptr ) std::cout << "***" << std::endl;

llvm::InitializeNativeTarget();
llvm::ExecutionEngine* exen = llvm::EngineBuilder(irmod).create();
std::vector<llvm::GenericValue> noargs;
llvm::GenericValue gv = exen->runFunction(ep, noargs);
}

