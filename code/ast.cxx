
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>

#include "ast.hxx"

/**/
namespace {
  /**/
  llvm::Type* asType(const std::string& id)
  {
    auto& cx = llvm::getGlobalContext();
    if( "Integer" == id )
      return llvm::Type::getInt32Ty(cx);
    if( "Double" == id )
      return llvm::Type::getDoubleTy(cx);
    if( "Boolean" == id )
      return llvm::Type::getInt1Ty(cx);
    return llvm::Type::getVoidTy(cx);
  }
}

/**/
Function::Function(const std::string& n, const std::map<std::string,std::string>& a, 
		   const std::string& t, Statement* b, llvm::Module* m)
  : name{n}, args{a}, type{t}, body{b}, module{m}
{
  body->setEnv(this);
}

/**/
void Function::Generate()
{
  std::vector<llvm::Type*> atypes;
  for( auto& a : args ) atypes.push_back(asType(a.second));
  auto rtype = asType(type);
  auto ftype = llvm::FunctionType::get(rtype, atypes, false);
  auto func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, module);
  auto ai = func->arg_begin();
  for( auto& a : args ) {
    ai->setName(a.first);
    ++ai;
  }
  auto block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "start", func);
  llvm::IRBuilder<> builder(llvm::getGlobalContext());
  builder.SetInsertPoint(block);

  for( auto ai = func->arg_begin(); ai != func->arg_end(); ++ai ) {
    auto s = builder.CreateAlloca(ai->getType());
    builder.CreateStore(ai, s);
    locals[ai->getName().str()] = s;
  }

  body->code(builder);
}

/**/
llvm::Value* Variable::code(llvm::IRBuilder<>& bu)
{
  return env->locals[name];
}

/**/
llvm::Value* Boolean::code(llvm::IRBuilder<>& bu)
{
  auto iv = llvm::APInt(1, value ? 1 : 0);
  return llvm::ConstantInt::get(llvm::getGlobalContext(), iv);
}

llvm::Value* Integer::code(llvm::IRBuilder<>& bu)
{
  auto iv = llvm::APInt(32, value, true);
  return llvm::ConstantInt::get(llvm::getGlobalContext(), iv);
}

/**/
llvm::Value* Double::code(llvm::IRBuilder<>& bu)
{
  auto fv = llvm::APFloat(value);
  return llvm::ConstantFP::get(llvm::getGlobalContext(), fv);
}

/**/
void Unary::setEnv(Function* e)
{
  Expression::setEnv(e);
  expr->setEnv(e);
}

/**/
llvm::Value* Unary::code(llvm::IRBuilder<>& bu)
{
  return nullptr; // TODO
}

void Binary::setEnv(Function* e)
{
  Expression::setEnv(e);
  expro->setEnv(e);
  expri->setEnv(e);
}

/**/
llvm::Value* Binary::code(llvm::IRBuilder<>& bu)
{
  return nullptr; // TODO
}

/**/
void Sequence::setEnv(Function* e) 
{
  Statement::setEnv(e);
  sto->setEnv(e);
  sti->setEnv(e);
}

/**/
void Sequence::code(llvm::IRBuilder<>& bu)
{
  sto->code(bu);
  sti->code(bu);
}

/**/
void Declare::code(llvm::IRBuilder<>& bu)
{
  env->locals[name] = bu.CreateAlloca(asType(type), nullptr, name);
}

/**/
void Result::setEnv(Function* e)
{
  Statement::setEnv(e);
  exp->setEnv(e);
}

/**/
void Result::code(llvm::IRBuilder<>& bu)
{
  bu.CreateRet(exp->code(bu));
}

/**/
void Assign::setEnv(Function* e)
{
  Statement::setEnv(e);
  expr->setEnv(e);
}

/**/
void Assign::code(llvm::IRBuilder<>& bu)
{
  auto d = env->locals[name];
  auto s = bu.CreateLoad(expr->code(bu));
  bu.CreateStore(s, d);
}


