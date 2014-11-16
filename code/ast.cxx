
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>

#include <llvm/ADT/ArrayRef.h>

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
Module::Module(const std::string& nm)
  : name{nm}
{
  module = new llvm::Module(name, llvm::getGlobalContext());
}

/**/
void Module::addFunction(Function* su)
{
  su->setModule(module);
  subs.push_back( su );
}

/**/
void Module::code(const std::string& on)
{
  // !!! temporary implementation !!!
  for( auto& e : subs ) {
    auto cc = e->code();
    cc->dump(); 
  }
}

/**/
Function::Function(const std::string& n, const vectorofpairsofstrings& a, const std::string& t)
  : name{n}, args{a}, type{t}, body{nullptr}, module{nullptr}
{}

/**/
void Function::setModule(llvm::Module* mo)
{ module = mo; }

/**/
void Function::setBody(Statement* bo)
{
  if( bo != nullptr ) {
    body = bo;
    body->setEnv(this);
  }
}

/**/
llvm::Function* Function::code()
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
  if( body == nullptr ) return func;

  auto block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "start", func);
  llvm::IRBuilder<> builder(llvm::getGlobalContext());
  builder.SetInsertPoint(block);

  for( auto ai = func->arg_begin(); ai != func->arg_end(); ++ai ) {
    auto s = builder.CreateAlloca(ai->getType());
    builder.CreateStore(ai, s);
    locals[ai->getName().str()] = s;
  }

  body->code(builder);
  return func;
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

/**/
void TypeCast::setEnv(Function* e)
{
  Expression::setEnv( e );
  expr->setEnv(e);
}

/**/
llvm::Value* TypeCast::code(llvm::IRBuilder<>& bu)
{
  auto& cx = llvm::getGlobalContext();
  auto exc = expr->code(bu);
  if( from == "Integer" && to == "Double" )
    return bu.CreateSIToFP( exc, llvm::Type::getDoubleTy(cx) );
  if( from == "Double" && to == "Integer" )
    return bu.CreateFPToSI( exc, llvm::Type::getInt32Ty(cx) );
  return exc;
}

/**/
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
void FuncCall::setEnv(Function* e)
{
  Expression::setEnv( e );
  for( auto& a : args ) a->setEnv( e );
}

/**/
llvm::Value* FuncCall::code(llvm::IRBuilder<>& bu)
{
  auto func = env->module->getFunction(name); // check
  std::vector<llvm::Value*> ags;
  for( auto& a : args )
    ags.push_back( a->code(bu) );
  return bu.CreateCall( func, llvm::ArrayRef<llvm::Value*>(ags) );
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
  //auto s = bu.CreateLoad(expr->code(bu)); // ?
  auto s = expr->code(bu); // corrections 
  bu.CreateStore(s, d);
}

/**/
void Branch::setEnv(Function* e)
{
}

/**/
void Branch::code(llvm::IRBuilder<>& bu)
{
}

/**/
void WhileLoop::setEnv(Function* e)
{
  Statement::setEnv(e);
  cond->setEnv(e);
  body->setEnv(e);
}

/**/
void WhileLoop::code(llvm::IRBuilder<>& bu)
{
}

/**/
void Input::code(llvm::IRBuilder<>& bu)
{
}

/**/
void Print::setEnv(Function* e)
{
  Statement::setEnv( e );
  for( auto& v : vals ) v->setEnv( e );
}

/**/
void code(llvm::IRBuilder<>& bu)
{
}


