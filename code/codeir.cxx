
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
    if( Expression::TyInteger == id )
      return llvm::Type::getInt32Ty(cx);
    if( Expression::TyDouble == id )
      return llvm::Type::getDoubleTy(cx);
    if( Expression::TyBoolean == id )
      return llvm::Type::getInt1Ty(cx);
    return llvm::Type::getVoidTy(cx);
  }
}

/**/
void Module::code(const std::string& eman)
{
  llvm::IRBuilder<> builder{llvm::getGlobalContext()};
  for( auto& e : subs ) e->code(builder);

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
llvm::Value* Constant::code(llvm::IRBuilder<>& bu)
{
  if( type == Expression::TyBoolean ) {
    auto iv = llvm::APInt{1, static_cast<unsigned long>(value == "True" ? 1 : 0)};
    return llvm::ConstantInt::get(llvm::getGlobalContext(), iv);
  }
  
  if( type == Expression::TyInteger ) {
    auto iv = llvm::APInt{32, static_cast<unsigned long>(std::stol(value)), true};
    return llvm::ConstantInt::get(llvm::getGlobalContext(), iv);
  }

  if( type == Expression::TyDouble ) {
    auto fv = llvm::APFloat{std::stod(value)};
    return llvm::ConstantFP::get(llvm::getGlobalContext(), fv);
  }
  
  return nullptr;
}

/**/
llvm::Value* Unary::code(llvm::IRBuilder<>& bu)
{
  auto exc = expr->code( bu );
  if( oper == "Not" )
    exc = bu.CreateNot(exc);
  else if( oper == "Neg" ) {
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
  auto& cx = llvm::getGlobalContext();
  auto exc = expr->code(bu);
  if( from == Expression::TyInteger && to == Expression::TyDouble )
    return bu.CreateSIToFP( exc, llvm::Type::getDoubleTy(cx) );
  if( from == Expression::TyDouble && to == Expression::TyInteger )
    return bu.CreateFPToSI( exc, llvm::Type::getInt32Ty(cx) );
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
  }
  else if( expro->type == Expression::TyBoolean && 
	   expri->type == Expression::TyBoolean ) {
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

  // կատարել param = e0 վերագրումը
  auto pr = env->locals[param];
  auto e0 = start->code(bu);
  if( e0->getType()->isPointerTy() )
    e0 = bu.CreateLoad(s);
  bu.CreateStore(e0, d);
  // հաշվել պարամետրի սահմանը
  auto e1 = stop->code(bu);
  // հաշվել պարամետրի քայլը
  auto e2 = step->code(bu);

  // կոնտեքստը և ընդգրկող ֆունկցիան
  auto& cx = llvm::getGlobalContext();
  auto subr = bu.GetInsertBlock()->getParent();

  auto cc = llvm::BasicBlock::Create( cx, "", subr ); // պայման
  auto cb = llvm::BasicBlock::Create( cx, "", subr ); // մարմին
  auto ce = llvm::BasicBlock::Create( cx, "", subr ); // ավարտ
 
  subr->getBasicBlockList().push_back(cc);
  bu.SetInsertPoint( cc );
 
  auto p0 = bu.CreateLoad(env->locals[param]);
  auto t0 = bu.CreateICmpSGT(e2, /*0*/);
  auto t1 = bu.CreateICmpSGT(p0, e1);
  auto t2 = bu.CreateAnd(t0, t1);
  auto p1 = bu.CreateLoad(env->locals[param]);
  auto t3 = bu.CreateICmpSLT(e2, /*0*/);
  auto t4 = bu.CreateICmpSLT(p0, e1);
  auto t5 = bu.CreateAnd(t0, t1);
  auto t6 = bu.CreateOr(t2, t5);
  auto tr = llvm::ConstantInt::get( cx, llvm::APInt(1, 1) );
  auto bv = bu.CreateICmpEQ( cv, tr );
  bu.CreateCondBr( bv, cb, ce );

  subr->getBasicBlockList().push_back(cb);
  bu.SetInsertPoint( cb );
  body->code( bu );
  auto p2 = bu.CreateLoad(env->locals[param]);
  auto t7 = bu.CreateNSWAdd(p2, e2);
  // TODO

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
    if( e->type == Expression::TyInteger )
      pr = env->module->getFunction("__printInteger__");
    else if( e->type == Expression::TyDouble )
      pr = env->module->getFunction("__printDouble__");
    else if( e->type == Expression::TyBoolean )
      pr = env->module->getFunction("__printBoolean__");      
    bu.CreateCall(pr, ec);
  }
  return nullptr;
}



