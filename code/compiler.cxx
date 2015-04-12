
#include <llvm/Linker/Linker.h>

#include <llvm/AsmParser/Parser.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>

#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>

#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>
#include <memory>

#include "ast.hxx"
#include "parser.hxx"
#include "compiler.hxx"
#include "support.hxx"

/**/
Module* compile(const std::string& name, bool lisp)
{
  // վերլուծություն
  Parser sc{name};
  auto moduleAst = sc.parse();
  if( nullptr == moduleAst ) return nullptr;

  // արտածել Լիսպ կոդը
  if( lisp ) {
    std::string basen{name};
    auto p = basen.find_last_of(".");
    if( p != std::string::npos ) basen.erase(p);
    std::ofstream slisp{basen + ".lisp"};
    moduleAst->lisp(slisp);
    slisp.close();
  }

  // LLVM IR կոդի գեներացիա
  auto& cox = llvm::getGlobalContext();
  llvm::IRBuilder<> builder{cox};
  moduleAst->code(builder);

  return moduleAst;
}

/**/
void generate(Module* moduleAst)
{
  // ստեղծվող ֆայլի անուն
  std::string basen{moduleAst->name};
  auto p = basen.find_last_of(".");
  if( p != std::string::npos ) basen.erase(p);
  std::string irout{basen + ".ll"};
  
  std::ofstream sout{irout};
  llvm::raw_os_ostream roo{sout};
  auto pmp = llvm::createPrintModulePass(roo, "");
  llvm::PassManager pm;
  pm.add(pmp);
  pm.run(*(moduleAst->getCompiled()));
  sout.close();
}

/**/
void jitRun(Module* moduleAst)
{
  /*
  llvm::SMDiagnostic er;
  auto suplib = llvm::parseAssemblyString(basic_ir_support_library, er, llvm::getGlobalContext());
  llvm::Linker link{suplib.get()};
  auto mo = moduleAst->getCompiled();
  link.linkInModule(mo);

  auto irmod = std::unique_ptr<llvm::Module>{link.getModule()};
  auto ep = irmod->getFunction("Main");
irmod->dump();  
  llvm::InitializeNativeTarget();
  llvm::ExecutionEngine* exen = llvm::EngineBuilder(std::move(irmod)).create();
  std::vector<llvm::GenericValue> noargs;
  llvm::GenericValue gv = exen->runFunction(ep, noargs);
  //delete exen;
  */
}

