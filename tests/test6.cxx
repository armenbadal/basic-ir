
#include <llvm/AsmParser/Parser.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/JITMemoryManager.h>

#include <iostream>
#include <string>


/**/
int main()
{
  llvm::SMDiagnostic er;
  auto module_0 = llvm::ParseAssemblyFile("ex3.ll", er, llvm::getGlobalContext());
  auto func = module_0->getFunction("main");
  //module_0->dump();

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::string error;
  auto exen = llvm::EngineBuilder(module_0)
    .setEngineKind(llvm::EngineKind::JIT)
    .setErrorStr(&error)
    .setJITMemoryManager(llvm::JITMemoryManager::CreateDefaultMemManager())
    .setUseMCJIT(true)
    .create();
  if( exen == nullptr )
    std::cout << "Cannot create ExecutionEngine" << std::endl;

  std::vector<llvm::GenericValue> noargs;
  auto res = exen->runFunction(func, noargs);
}

