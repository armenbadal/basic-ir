
#include <llvm/Linker.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

#include <string>

const char* f_pow{
  "define i32 @__i_pow__() nounwind uwtable {\n"
  "  ret i32 777\n"
  "}\n"
};

/**/
int main()
{
  llvm::SMDiagnostic er;
  auto module_0 = llvm::ParseAssemblyFile("ex0.ll", er, llvm::getGlobalContext());
  auto module_1 = llvm::ParseAssemblyFile("ex1.ll", er, llvm::getGlobalContext());
  auto module_2 = llvm::ParseAssemblyString(f_pow, module_1, er, llvm::getGlobalContext());

  llvm::Linker link{module_1};
  link.linkInModule(module_0, llvm::Linker::PreserveSource, nullptr);
  link.linkInModule(module_2, llvm::Linker::PreserveSource, nullptr);
  auto allm = link.getModule();
  allm->dump();

  //  module_0->dump();
  //  module_1->dump();
}

