
#include "ast.hxx"

/**/
Module::Module(const std::string& nm)
  : name{nm}
{
  module = new llvm::Module(name, llvm::getGlobalContext());

  // գրադարանային ֆունկցիաներ
  // արտածում
  auto e0 = new Function{"__printInteger__", vectornametype{{"_",Expression::TyInteger}}, Expression::TyVoid};
  auto e1 = new Function{"__printDouble__", vectornametype{{"_",Expression::TyDouble}}, Expression::TyVoid};
  auto e2 = new Function{"__printBoolean__", vectornametype{{"_",Expression::TyBoolean}}, Expression::TyVoid};
  addFunction(e0); addFunction(e1); addFunction(e2);
}

/**/
Module::~Module()
{
  for( auto em : subs ) delete em;
  delete module;
}

/* ?? */
void Module::addFunction(Function* su)
{
  su->setModule(module);
  subs.push_back( su );
}

/**/
Function::Function(const std::string& n, const vectornametype& a, const std::string& t)
  : name{n}, args{a}, type{t}
{}

/**/
Function::~Function()
{
  delete body;
}

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
const std::string Expression::TyBoolean{"BOOLEAN"};
const std::string Expression::TyInteger{"INTEGER"};
const std::string Expression::TyDouble{"DOUBLE"};
const std::string Expression::TyVoid{"VOID"};

/**/
Unary::Unary(const std::string& op, Expression* ex)
  : oper{op}, expr{ex}
{
  if( oper == "Neg" ) 
     type = expr->type;
  else if( oper == "Not" )
    type = Expression::TyBoolean;
}

/**/
void Unary::setEnv(Function* e)
{
  Expression::setEnv(e);
  expr->setEnv(e);
}

/**/
void TypeCast::setEnv(Function* e)
{
  Expression::setEnv( e );
  expr->setEnv(e);
}

/**/
std::set<std::string> Binary::Numerics{"+", "-", "*", "/", "\\", "^"};
std::set<std::string> Binary::Logicals{"And", "Or", "=", "<>", ">", ">=", "<", "<="};

/**/
Binary::Binary(const std::string& op, Expression* exo, Expression* exi)
  : oper{op}, expro{exo}, expri{exi} 
{
  // տիպերի համաձայնեցում և ձևափոխում
  if( expro->type == Expression::TyInteger && expri->type == Expression::TyDouble )
    expro = new TypeCast{expro, Expression::TyInteger, Expression::TyDouble};
  else if( expro->type == Expression::TyDouble && expri->type == Expression::TyInteger )
    expri = new TypeCast{expri, Expression::TyInteger, Expression::TyDouble};

  // տիպի դուրսբերում
  if( Numerics.end() != Numerics.find(oper) )
    if( expro->type == Expression::TyInteger && expri->type == Expression::TyInteger )
      type = Expression::TyInteger;
    else
      type = Expression::TyDouble;
  else if( Logicals.end() !=  Logicals.find(oper) )
    type = Expression::TyBoolean;
}

/**/
Binary::~Binary()
{
  delete expro;
  delete expri;
}

/**/
void Binary::setEnv(Function* e)
{
  Expression::setEnv(e);
  expro->setEnv(e);
  expri->setEnv(e);
}

/**/
FuncCall::~FuncCall()
{
  for( auto e : args ) delete e; 
}

/**/
void FuncCall::setEnv(Function* e)
{
  Expression::setEnv( e );
  for( auto& a : args ) a->setEnv( e );
}

/**/
Sequence::~Sequence() 
{
  delete sto;
  delete sti;
}

/**/
void Sequence::setEnv(Function* e) 
{
  Statement::setEnv(e);
  sto->setEnv(e);
  sti->setEnv(e);
}

/**/
SubCall::~SubCall()
{
  delete subr;
}

/**/
void SubCall::setEnv(Function* e)
{
  Statement::setEnv(e);
  subr->setEnv(e);
}

/**/
Result::~Result()
{
  delete exp;
}

/**/
void Result::setEnv(Function* e)
{
  Statement::setEnv(e);
  exp->setEnv(e);
}

/**/
Assign::~Assign()
{
  delete expr;
}

/**/
void Assign::setEnv(Function* e)
{
  Statement::setEnv(e);
  expr->setEnv(e);
}

/**/
Branch::~Branch()
{
  delete cond;
  delete thenp;
  delete elsep;
}

/**/
void Branch::setEnv(Function* e)
{
  Statement::setEnv( e );
  cond->setEnv( e );
  thenp->setEnv( e );
  if( elsep != nullptr ) elsep->setEnv( e );
}

/**/
ForLoop::~ForLoop() 
{
  delete start;
  delete stop;
  delete step;
  delete body;
}

/**/
void ForLoop::setEnv(Function* e)
{
  Statement::setEnv( e );
  start->setEnv( e );
  stop->setEnv( e );
  step->setEnv( e );
  body->setEnv( e );
}

/**/
WhileLoop::~WhileLoop()
{
  delete cond;
  delete body;
}

/**/
void WhileLoop::setEnv(Function* e)
{
  Statement::setEnv(e);
  cond->setEnv(e);
  body->setEnv(e);
}

/**/
Print::~Print()
{
  for( auto v : vals ) delete v; 
}

/**/
void Print::setEnv(Function* e)
{
  Statement::setEnv( e );
  for( auto& v : vals ) v->setEnv( e );
}

