
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>

#include <llvm/ADT/ArrayRef.h>

#include <llvm/Support/raw_os_ostream.h>

#include <fstream>

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
void Module::code(const std::string& eman)
{
  llvm::IRBuilder<> builder{llvm::getGlobalContext()};
  for( auto& e : subs ) e->code(builder);

  /* EXPERIMENT */ llvm::GlobalVariable vr{*module, llvm::Type::getDoubleTy(llvm::getGlobalContext()), true, llvm::GlobalVariable::PrivateLinkage, nullptr, "gv0"};
  vr.dump();
  /* DEBUG */ module->dump();

  std::ofstream sout{eman};
  llvm::raw_os_ostream roo{sout};
  llvm::PrintModulePass pmp{roo};
  llvm::ModulePassManager pm;
  pm.addPass(pmp);
  pm.run(module);
  sout.close();
}

/**/
llvm::Value* Function::code(llvm::IRBuilder<>& bu)
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
  bu.SetInsertPoint(block);

  for( auto ai = func->arg_begin(); ai != func->arg_end(); ++ai ) {
    auto s = bu.CreateAlloca(ai->getType());
    bu.CreateStore(ai, s);
    locals[ai->getName().str()] = s;
  }

  body->code(bu);
  if( type == "Void" ) bu.CreateRetVoid();
  return func;
}

/**/
llvm::Value* Variable::code(llvm::IRBuilder<>& bu)
{
  return bu.CreateLoad(env->locals[name]);
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
llvm::Value* Unary::code(llvm::IRBuilder<>& bu)
{
  auto exc = expr->code( bu );
  if( oper == "Not" )
    exc = bu.CreateNot(exc);
  else if( oper == "Neg" ) {
    if( type == "Integer" ) 
      exc = bu.CreateNeg(exc);
    else if( type == "Double" ) 
      exc = bu.CreateFNeg(exc);
  }
  return exc;
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
llvm::Value* Binary::code(llvm::IRBuilder<>& bu)
{
  auto exo = expro->code( bu ); // ձախ
  auto exi = expri->code( bu ); // աջ

  // գործողության երկու կողմերում իրական արժեք վերադարձնող արտահայտությունենր են
  if( expro->type == "Double" && expri->type == "Double" ) {
    if( oper == "=" ) 
      return bu.CreateFCmpOEQ(exo, exi); 
    if( oper == "<>" )
      return bu.CreateFCmpONE(exo, exi);
    if( oper == ">" )
      return bu.CreateFCmpOGT(exo, exi);
    if( oper == ">=" )
      return bu.CreateFCmpOGE(exo, exi);
    if( oper == "<" )
      return bu.CreateFCmpOLT(exo, exi);
    if( oper == "<=" )
      return bu.CreateFCmpOLE(exo, exi);
    if( oper == "+" )
      return bu.CreateFAdd(exo, exi);
    if( oper == "-" )
      return bu.CreateFSub(exo, exi);
    if( oper == "*" )
      return bu.CreateFMul(exo, exi);
    if( oper == "/" )
      return bu.CreateFDiv(exo, exi);
    if( oper == "\\" )
      return bu.CreateFRem(exo, exi);
  }
  else if( expro->type == "Integer" && expri->type == "Integer" ) {
    if( oper == "=" ) 
      return bu.CreateICmpEQ(exo, exi); 
    if( oper == "<>" )
      return bu.CreateICmpNE(exo, exi);
    if( oper == ">" )
      return bu.CreateICmpSGT(exo, exi);
    if( oper == ">=" )
      return bu.CreateICmpSGE(exo, exi);
    if( oper == "<" )
      return bu.CreateICmpSLT(exo, exi);
    if( oper == "<=" )
      return bu.CreateICmpSLE(exo, exi);
    if( oper == "+" )
      return bu.CreateNSWAdd(exo, exi);
    if( oper == "-" )
      return bu.CreateNSWSub(exo, exi);
    if( oper == "*" )
      return bu.CreateNSWMul(exo, exi);
    if( oper == "/" )
      return bu.CreateSDiv(exo, exi);
    if( oper == "\\" )
      return bu.CreateSRem(exo, exi);
  }
  else if( expro->type == "Boolean" && expri->type == "Boolean" ) {
    if( oper == "And" ) 
      return bu.CreateAnd(exo, exi);
    if( oper == "Or" ) 
      return bu.CreateOr(exo, exi);
    if( oper == "=" ) 
      return bu.CreateICmpEQ(exo, exi); 
    if( oper == "<>" )
      return bu.CreateICmpNE(exo, exi);
  }

  return nullptr;
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
llvm::Value* Sequence::code(llvm::IRBuilder<>& bu)
{
  sto->code(bu);
  sti->code(bu);
  return nullptr;
}

/**/
llvm::Value* Declare::code(llvm::IRBuilder<>& bu)
{
  env->locals[name] = bu.CreateAlloca(asType(type), nullptr, name);
  return nullptr;
}

/**/
llvm::Value* Result::code(llvm::IRBuilder<>& bu)
{
  return bu.CreateRet(exp->code(bu));
}

/**/
llvm::Value* Assign::code(llvm::IRBuilder<>& bu)
{
  auto d = env->locals[name];
  auto s = expr->code(bu);
  if( s->getType()->isPointerTy() )
    s = bu.CreateLoad(s);
  bu.CreateStore(s, d);
  return nullptr;
}

/**/
llvm::Value* Branch::code(llvm::IRBuilder<>& bu)
{
  auto& cx = llvm::getGlobalContext();
  auto func = bu.GetInsertBlock()->getParent();

  auto cv = cond->code( bu );
  auto bv = bu.CreateICmpEQ( cv, llvm::ConstantInt::get( cx, llvm::APInt(1, 1) ) );

  auto tb = llvm::BasicBlock::Create( cx, "", func );
  auto cb = llvm::BasicBlock::Create( cx, "", func );
  auto eb = cb;
  if( elsep != nullptr )
    eb = llvm::BasicBlock::Create( cx, "", func );
  
  bu.CreateCondBr( bv, tb, eb );
  
  func->getBasicBlockList().push_back(tb);
  bu.SetInsertPoint( tb );
  thenp->code( bu );
  bu.CreateBr( cb );
  
  if( elsep != nullptr ) {
    func->getBasicBlockList().push_back(eb);
    bu.SetInsertPoint( eb );
    elsep->code( bu );
    bu.CreateBr( cb );
  }

  func->getBasicBlockList().push_back(cb);
  bu.SetInsertPoint( cb );

  return nullptr;
}

/**/
llvm::Value* ForLoop::code(llvm::IRBuilder<>&)
{
  return nullptr;
}

/**/
llvm::Value* WhileLoop::code(llvm::IRBuilder<>& bu)
{
  auto& cx = llvm::getGlobalContext();
  auto func = bu.GetInsertBlock()->getParent();

  auto wc = llvm::BasicBlock::Create( cx, "", func ); // cond
  auto wb = llvm::BasicBlock::Create( cx, "", func ); // body
  auto we = llvm::BasicBlock::Create( cx, "", func ); // end

  func->getBasicBlockList().push_back(wc);
  bu.SetInsertPoint( wc );
  auto cv = cond->code( bu );
  auto tr = llvm::ConstantInt::get( cx, llvm::APInt(1, 1) );
  auto bv = bu.CreateICmpEQ( cv, tr );
  bu.CreateCondBr( bv, wb, we );

  func->getBasicBlockList().push_back(wb);
  bu.SetInsertPoint( wb );
  body->code( bu );
  bu.CreateBr( wc );

  func->getBasicBlockList().push_back(we);
  bu.SetInsertPoint( we );

  return nullptr;
}

/**/
llvm::Value* Input::code(llvm::IRBuilder<>& bu)
{
  return nullptr;
}

/**/
llvm::Value* Print::code(llvm::IRBuilder<>& bu)
{
  llvm::Function* pr{nullptr};
  for( auto e : vals ) {
    auto ec = e->code(bu);
    if( e->type == "Integer" )
      pr = env->module->getFunction("__printInteger__");
    else if( e->type == "Double" )
      pr = env->module->getFunction("__printDouble__");
    else if( e->type == "Boolean" )
      pr = env->module->getFunction("__printBoolean__");      
    bu.CreateCall(pr, ec);
  }
  return nullptr;
}



