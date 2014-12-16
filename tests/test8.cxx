
#include <llvm/AsmParser/Parser.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;

int main() 
{
  
  InitializeNativeTarget();

  LLVMContext& Context = getGlobalContext();
  
  llvm::SMDiagnostic er;
  auto M = llvm::ParseAssemblyFile("ex3.ll", er, Context);
  auto F = M->getFunction("main");

  ExecutionEngine* EE = EngineBuilder(M).create();

  std::vector<GenericValue> noargs;
  GenericValue gv = EE->runFunction(F, noargs);

  outs() << "Result: " << gv.IntVal << "\n";
  EE->freeMachineCodeForFunction(F);
  delete EE;
}

