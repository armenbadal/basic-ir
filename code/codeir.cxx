
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

#include <llvm/ADT/ArrayRef.h>

#include <fstream>
#include <iostream>

#include "ast.hxx"

/**/
namespace {
  /**/
  auto& context = llvm::getGlobalContext();
  /**/
  const auto I = llvm::Type::getInt32Ty(context);
  const auto D = llvm::Type::getDoubleTy(context);
  const auto B = llvm::Type::getInt1Ty(context);
  const auto V = llvm::Type::getVoidTy(context);
  /**/
  llvm::Type* asType(const std::string& id)
  {
    if( Expression::TyInteger == id ) return I;
    if( Expression::TyDouble == id ) return D;
    if( Expression::TyBoolean == id ) return B;
    return V;
  }
  /**/
  llvm::FunctionType* signature(const std::vector<llvm::Type*>& tyargs, llvm::Type* tyret)
  {
    return llvm::FunctionType::get(tyret, tyargs, false);
  }
}

/**/
llvm::Value* Module::code(llvm::IRBuilder<>& bu)
{
  module = new llvm::Module( name, context );

  // սահմանված ենթածրագրեր
  for( auto& e : subroutines ) { 
    e->setModule(module);
    e->code(bu);
  }

  llvm::verifyModule(*module);
  return nullptr;
}

/**/
llvm::Value* Function::code(llvm::IRBuilder<>& bu)
{
  std::vector<llvm::Type*> atypes;
  for( auto& a : args ) atypes.push_back(asType(a.second));
  auto rtype = asType(type);
  auto ftype = llvm::FunctionType::get(rtype, atypes, false);
  auto func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, module);
  // ստուգել կոնֆլիկտները 
  if( name != func->getName() ) {
    func->eraseFromParent();
    func = module->getFunction(name);
  }
  auto ai = func->arg_begin();
  for( auto& a : args ) {
    ai->setName(a.first);
    ++ai;
  }
  if( body == nullptr ) return func;

  auto block = llvm::BasicBlock::Create(context, "start", func);
  bu.SetInsertPoint(block);

  for( auto ai = func->arg_begin(); ai != func->arg_end(); ++ai ) {
    auto s = bu.CreateAlloca(ai->getType());
    bu.CreateStore(ai, s);
    locals[ai->getName().str()] = s;
  }

  body->code(bu);
  if( type == Expression::TyVoid ) bu.CreateRetVoid();

  llvm::verifyFunction(*func);
  return func;
}

/**/
llvm::Value* Variable::code(llvm::IRBuilder<>& bu)
{
  return bu.CreateLoad(env->locals[name]);
}

/**/
llvm::Value* Constant::code(llvm::IRBuilder<>& bu)
{
  if( type == Expression::TyBoolean ) {
    auto iv = llvm::APInt{1, static_cast<unsigned long>(value == "true" ? 1 : 0)};
    return llvm::ConstantInt::get(context, iv);
  }
  
  if( type == Expression::TyInteger ) {
    auto iv = llvm::APInt{32, static_cast<unsigned long>(std::stol(value)), true};
    return llvm::ConstantInt::get(context, iv);
  }

  if( type == Expression::TyDouble ) {
    auto fv = llvm::APFloat{std::stod(value)};
    return llvm::ConstantFP::get(context, fv);
  }
  
  return nullptr;
}

/**/
llvm::Value* Unary::code(llvm::IRBuilder<>& bu)
{
  auto exc = expr->code( bu );
  if( oper == "not" )
    exc = bu.CreateNot(exc);
  else if( oper == "neg" ) {
    if( type == Expression::TyInteger ) 
      exc = bu.CreateNeg(exc);
    else if( type == Expression::TyDouble ) 
      exc = bu.CreateFNeg(exc);
  }
  return exc;
}

/**/
llvm::Value* TypeCast::code(llvm::IRBuilder<>& bu)
{
  auto exc = expr->code(bu);
  if( to == Expression::TyDouble )
    return bu.CreateSIToFP( exc, llvm::Type::getDoubleTy(context) );
  if( to == Expression::TyInteger )
    return bu.CreateFPToSI( exc, llvm::Type::getInt32Ty(context) );
  return exc;
}

/**/
llvm::Value* Binary::code(llvm::IRBuilder<>& bu)
{
  auto exo = expro->code( bu ); // ձախ
  auto exi = expri->code( bu ); // աջ

  // գործողության երկու կողմերում իրական արժեք վերադարձնող արտահայտությունենր են
  if( expro->type == Expression::TyDouble && 
      expri->type == Expression::TyDouble ) {
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
    if( oper == "^" ) {
      auto pw = env->module->getOrInsertFunction("llvm.pow.f64", signature({D,D}, D));
      return bu.CreateCall2(pw, exo, exi);
    }
  }
  else if( expro->type == Expression::TyInteger && 
	   expri->type == Expression::TyInteger ) {
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
    if( oper == "^" ) {
      exo = bu.CreateSIToFP( exo, D );
      auto pw = env->module->getOrInsertFunction("llvm.powi.f64", signature({D,I}, D));
      auto r0 = bu.CreateCall2( pw, exo, exi );
      return bu.CreateFPToSI( r0, I );
    }
  }
  else if( expro->type == Expression::TyBoolean && 
	   expri->type == Expression::TyBoolean ) {
    if( oper == "and" ) 
      return bu.CreateAnd(exo, exi);
    if( oper == "or" ) 
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
  auto func = env->module->getFunction(name); // ստուգել
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
llvm::Value* SubCall::code(llvm::IRBuilder<>& bu)
{
  subr->code(bu);
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
  auto func = bu.GetInsertBlock()->getParent();

  auto cv = cond->code( bu );
  auto bv = bu.CreateICmpEQ( cv, llvm::ConstantInt::get( context, llvm::APInt(1, 1) ) );

  auto tb = llvm::BasicBlock::Create( context, "", func );
  auto cb = llvm::BasicBlock::Create( context, "", func );
  auto eb = cb;
  if( elsep != nullptr )
    eb = llvm::BasicBlock::Create( context, "", func );
  
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

/*
  FOR param = e0 TO e1 [STEP e2]
    body
  END FOR
*/
llvm::Value* ForLoop::code(llvm::IRBuilder<>& bu)
{
  /*
    ' համարժեք գրառումը WHILE ցիկլի օգտագործմամբ
    param = e0
    DIM __e1 AS <type of param>
    __e1 = e1
    DIM __e2 AS <type of param>
    __e2 = e2
    WHILE __e2 > 0 AND param < __e1 OR __e2 < 0 AND param > __e1
      body
      param = param + __e2
    END WHILE
  */

  // կոնտեքստը և ընդգրկող ֆունկցիան
  auto subr = bu.GetInsertBlock()->getParent();

  auto cc = llvm::BasicBlock::Create( context, "", subr ); // պայման
  auto cb = llvm::BasicBlock::Create( context, "", subr ); // մարմին
  auto ce = llvm::BasicBlock::Create( context, "", subr ); // ավարտ

  // կատարել param = e0 վերագրումը
  auto pr = env->locals[param];
  auto e0 = start->code(bu);
  if( e0->getType()->isPointerTy() )
    e0 = bu.CreateLoad(e0);
  bu.CreateStore(e0, pr);
  // հաշվել պարամետրի սահմանը
  auto e1 = stop->code(bu);
  // հաշվել պարամետրի քայլը
  auto e2 = step->code(bu);
  bu.CreateBr( cc );

  subr->getBasicBlockList().push_back(cc);
  bu.SetInsertPoint( cc );

  // ցիկլի ավարտի պայման e2 > 0 & p0 > e1 | e2 < 0 & p0 < e1
  auto p0 = bu.CreateLoad(pr);
  auto z0 = llvm::ConstantInt::get(context, llvm::APInt{32, 0, true});
  auto t0 = bu.CreateICmpSGT(e2, z0);
  auto t1 = bu.CreateICmpSGT(p0, e1);
  auto t2 = bu.CreateAnd(t0, t1);
  auto t3 = bu.CreateICmpSLT(e2, z0);
  auto t4 = bu.CreateICmpSLT(p0, e1);
  auto t5 = bu.CreateAnd(t3, t4);
  auto t6 = bu.CreateOr(t2, t5);
  auto tr = llvm::ConstantInt::get(context, llvm::APInt{1, 0});
  auto bv = bu.CreateICmpEQ(t6, tr);
  bu.CreateCondBr( bv, cb, ce );

  subr->getBasicBlockList().push_back(cb);
  bu.SetInsertPoint( cb );
  body->code( bu );
  auto p1 = bu.CreateLoad(pr);
  auto t7 = bu.CreateNSWAdd(p1, e2);
  bu.CreateStore(t7, pr);
  bu.CreateBr( cb );
  subr->getBasicBlockList().push_back(ce);
  bu.SetInsertPoint( ce );

  return nullptr;
}

/**/
llvm::Value* WhileLoop::code(llvm::IRBuilder<>& bu)
{
  auto func = bu.GetInsertBlock()->getParent();

  auto wc = llvm::BasicBlock::Create( context, "", func ); // cond
  auto wb = llvm::BasicBlock::Create( context, "", func ); // body
  auto we = llvm::BasicBlock::Create( context, "", func ); // end

  // bu.CreateBr( cx ); // TODO

  func->getBasicBlockList().push_back(wc);
  bu.SetInsertPoint( wc );
  auto cv = cond->code( bu );
  auto tr = llvm::ConstantInt::get( context, llvm::APInt(1, 1) );
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
  for( auto& sy : vars ) {
    llvm::Value* pr{nullptr};
    if( sy.second == Expression::TyInteger )
      pr = env->module->getOrInsertFunction("__input_integer__", signature({}, I));
    else if( sy.second == Expression::TyDouble )
      pr = env->module->getOrInsertFunction("__input_double__", signature({}, D));
    else if( sy.second == Expression::TyBoolean )
      pr = env->module->getOrInsertFunction("__input_boolean__", signature({}, B));
    auto vl = bu.CreateCall(pr);
    auto ds = env->locals[sy.first];
    bu.CreateStore(vl, ds);
  }
  return nullptr;
}

/**/
llvm::Value* Print::code(llvm::IRBuilder<>& bu)
{
  const auto space = llvm::ConstantInt::get(context, llvm::APInt{32, 32});
  for( auto e : vals ) {
    auto ec = e->code(bu);
    llvm::Value* pr{nullptr};
    if( e->type == Expression::TyInteger )
      pr = env->module->getOrInsertFunction("__print_integer__", signature({I}, V));
    else if( e->type == Expression::TyDouble )
      pr = env->module->getOrInsertFunction("__print_double__", signature({D}, V));
    else if( e->type == Expression::TyBoolean )
      pr = env->module->getOrInsertFunction("__print_boolean__", signature({B}, V));
    bu.CreateCall(pr, ec);
    bu.CreateCall(env->module->getOrInsertFunction("putchar", signature({I}, I)), space);
  }
  const auto newli = llvm::ConstantInt::get(context, llvm::APInt{32, 10});
  bu.CreateCall(env->module->getOrInsertFunction("putchar", signature({I}, I)), newli);
  return nullptr;
}



