
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "parser.hxx"

namespace {
  /**/
  std::vector<std::string> TN = {
    "NIL", "EOL", "Integer", "Double", "True", "False", 
    "Ident", "Dim", "As", "Type", "End", "Declare", "Sub", 
    "Function", "Return", "If", "Then", "ElseIf", "Else",
    "For", "To", "Step", "While", "Input", "Print",
    "(", ")", ",", "And", "Or", "Not", "=", "<>", ">", 
    ">=", "<", "<=", "+", "-", "*", "/", "\\", "^", "EOF"
  };
  /**/
  bool isNumeric(const std::string& id)
  {
    return id == Expression::TyInteger || id == Expression::TyDouble;
  }
}

/**/
void Parser::match( Token exp )
{
  if( lookahead == exp )
    lookahead = sc.next();
  else
    throw new std::logic_error{"ՍԽԱԼ։ Սպասվում էր `" + TN[exp] + 
	"', բայց եղել է `" + TN[lookahead] + "'։"};
}

/**/
void Parser::parseEols()
{
  while( lookahead == xEol )
    lookahead = sc.next();
}

/* FIRST sets */
std::set<Token> Parser::FD{ xDim, xType, xDeclare, xSubroutine, xFunction };
std::set<Token> Parser::FS{ xDim, xIdent, xIf, xFor, xWhile, xReturn, xInput, xPrint };
std::set<Token> Parser::FF{ xIdent, xInteger, xDouble, xTrue, xFalse, xSub, xNot, xLPar };

/**/
bool Parser::inSet( const std::set<Token>& es )
{
  return es.end() != es.find(lookahead);
}

/* Այստեղից սկսվում է վերլուծությունը */
Module* Parser::parse()
{
  Module* mod = new Module{file};

  do
    lookahead = sc.next();
  while( lookahead == xEol );

  symtab->openScope(); // ?

  try {
    while( inSet(FD) ) {
      Function* sub{nullptr};
      if( lookahead == xDeclare )
	sub = parseDeclare();
      else if( lookahead == xFunction )
	sub = parseFunction();
      else if( lookahead == xSubroutine )
	sub = parseSubroutine();
      mod->addFunction(sub);
    }
  }
  catch( std::exception* e ) {
    std::cerr << "ՍԽԱԼ [" << sc.line() << "]: " << e->what() << std::endl;
    delete e; delete mod;
    return nullptr;
  }

  /* DEBUG */ std::cout << "PARSED" << std::endl;
  return mod;
}

/**/
Statement* Parser::parseStatement()
{
  if( lookahead == xDim )
    return parseDim();

  if( lookahead == xIdent )
    return parseSubCallOrAssign();

  if( lookahead == xIf )
    return parseIf();

  if( lookahead == xFor )
    return parseFor();

  if( lookahead == xWhile )
    return parseWhile();

  if( lookahead == xInput )
    return parseInput();

  if( lookahead == xPrint )
    return parsePrint();

  if( lookahead == xReturn )
    return parseReturn();

  return nullptr;
}

/**/
Statement* Parser::parseSequence()
{
  Statement* res{nullptr};
  while( inSet(FS) )
    if( res == nullptr )
      res = parseStatement();
    else
      res = new Sequence{res, parseStatement()};
  return res;
}

/**/
Symbol Parser::parseNameDecl()
{
  auto a0 = sc.lexeme();
  match( xIdent );
  match( xAs );
  auto a1 = sc.lexeme();
  match( xIdent );
  return std::make_pair(a0, a1);
}

/**/
std::string Parser::parseDeclList(std::vector<Symbol>& ds)
{
  std::stringstream ss;
  auto nv = parseNameDecl();
  ds.push_back( nv );
  ss << nv.second;
  while( lookahead == xComma ) {
    match( xComma );
    nv = parseNameDecl();
    ds.push_back( nv );
    ss << " x " << nv.second;
  }
  return ss.str();
}

/**/
std::string Parser::parseArguments(std::vector<Expression*>& es)
{
  std::stringstream spec;
  if( inSet(FF) ) {
    auto ex = parseDisjunction();
    spec << ex->type;
    es.push_back( ex );
    while( lookahead == xComma ) {
      lookahead = sc.next();
      ex = parseDisjunction();
      spec << " x " << ex->type;
      es.push_back( ex );
    }
  }
  return spec.str();
}

/**/
void Parser::parseType()
{
  match( xType );
  std::string nm = sc.lexeme();
  match( xIdent );
  match( xEol );
  while( lookahead == xIdent ) {
    parseNameDecl();
    match( xEol );
  }
  match( xEnd );
  match( xType );
}

/**/
Function* Parser::parseDeclare()
{
  Function* result{nullptr};
  match( xDeclare );
  if( lookahead == xSubroutine )
    result = parseSubrHeader();
  else if( lookahead == xFunction )
    result = parseFuncHeader();
  return result;
}

/**/
Function* Parser::parseSubrHeader()
{
  match( xSubroutine );
  std::string nm = sc.lexeme();
  match( xIdent );
  std::string sig{"()"};
  std::vector<Symbol> ag;
  if( lookahead == xIdent )
    sig = parseDeclList( ag );
  parseEols();
  if( nm == "Main" && ag.size() != 0 )
    throw new std::logic_error{"'Main' պրոցեդուրան արգումենտներ չունի։"};
  symtab->insert( Symbol{nm, sig + " -> Void"} );
  return new Function{nm, ag, "Void"};
}

/**/
Function* Parser::parseSubroutine()
{
  auto pr = parseSubrHeader();
  symtab->openScope();
  for( auto& a : pr->args )
    symtab->insert( Symbol{a.first, a.second} );
  auto bo = parseSequence();
  match( xEnd );
  match( xSubroutine );
  parseEols();
  pr->setBody( bo );
  symtab->closeScope();
  return pr;
}

/**/
Function* Parser::parseFuncHeader()
{
  match( xFunction );
  std::string nm = sc.lexeme();
  match( xIdent );
  if( nm == "Main" )
    throw new std::logic_error{"'Main'-ը պետք է լինի պրոցեդուրա։"};
  match( xLPar );
  std::string sig{"()"};
  std::vector<Symbol> ag;
  if( lookahead == xIdent )
    sig = parseDeclList( ag );
  match( xRPar );
  match( xAs );
  std::string ty = sc.lexeme();
  match( xIdent );
  parseEols();
  symtab->insert( Symbol{nm, sig + " -> " + ty} );
  return new Function{nm, ag, ty};
}

/**/
Function* Parser::parseFunction()
{
  auto pr = parseFuncHeader();
  symtab->openScope();
  for( auto& a : pr->args )
    symtab->insert( Symbol{a.first, a.second} );
  auto bo = parseSequence();
  match( xEnd );
  match( xFunction );
  parseEols();
  pr->setBody( bo );
  symtab->closeScope();
  return pr;
}

/**/
Statement* Parser::parseReturn()
{
  match( xReturn );
  auto rv = parseDisjunction();
  parseEols();
  return new Result{rv};
}

/**/
Statement* Parser::parseDim()
{
  match( xDim );
  auto nv = parseNameDecl();
  parseEols();
  symtab->insert(nv);
  return new Declare{nv.first, nv.second};
}

/**/
Statement* Parser::parseSubCallOrAssign()
{
  // փոփոխականի կամ պրոցեդուրայի անուն
  auto vn = sc.lexeme();
  match( xIdent );
  auto nt = symtab->search(vn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // վերագրման հրաման
  if( lookahead == xEq ) {
    lookahead = sc.next();
    auto ex = parseDisjunction();
    parseEols();
    return new Assign{vn, ex};    
  }

  // պրոցեդուրայի կանչի արգումենտներ
  std::vector<Expression*> es;
  auto ty = parseArguments( es );
  parseEols();

  return new SubCall{vn, es};
}

/**/
Statement* Parser::parseIf()
{
  match( xIf );
  auto cond = parseDisjunction();
  match( xThen );
  parseEols();
  auto thenp = parseSequence();
  auto branch = new Branch{cond, thenp, nullptr};
  auto brit = branch;
  while( lookahead == xElseIf ) {
    match( xElseIf );
    cond = parseDisjunction();
    match( xThen );
    parseEols();
    thenp = parseSequence();
    auto elsep = new Branch( cond, thenp, nullptr );
    brit->setElse( elsep );
    brit = elsep;
  }
  if( lookahead == xElse ) {
    match( xElse );
    parseEols();
    auto elsep = parseSequence();
    brit->setElse( elsep );
  }
  match( xEnd );
  match( xIf );
  parseEols();
  return branch;
}

/**/
Statement* Parser::parseFor()
{
  match( xFor );
  auto cn = sc.lexeme();
  match( xIdent );
  // ստուգել cn-ի տիպը․ պետք է լինի Integer կամ Double
  match( xEq );
  auto st = parseAddition();
  match( xTo );
  auto ed = parseAddition();
  Expression* sp{nullptr};
  if( lookahead == xStep ) {
    match( xStep );
    sp = parseAddition();
  }
  parseEols();
  auto bo = parseSequence();
  match( xEnd );
  match( xFor );
  parseEols();
  return new ForLoop{cn, st, ed, sp, bo};
}

/**/
Statement* Parser::parseWhile()
{
  match( xWhile );
  auto cond = parseDisjunction();
  parseEols();
  auto body = parseSequence();
  match( xEnd );
  match( xWhile );
  parseEols();
  return new WhileLoop{cond, body};
}

/**/
Statement* Parser::parseInput()
{
  match( xInput );
  std::vector<std::string> vars;
  auto nm = sc.lexeme();
  match( xIdent );
  vars.push_back( nm );
  while( lookahead == xComma ) {
    lookahead = sc.next();
    nm = sc.lexeme();
    match( xIdent );
    vars.push_back( nm );
  }
  parseEols();
  return new Input{vars};
}

/**/
Statement* Parser::parsePrint()
{
  match( xPrint );
  std::vector<Expression*> vals;
  auto ex =  parseDisjunction();
  vals.push_back( ex );
  while( lookahead == xComma ) {
    lookahead = sc.next();
    ex = parseDisjunction();
    vals.push_back( ex );
  }
  parseEols();
  return new Print{vals};
}

/**/
Expression* Parser::parseDisjunction()
{
  auto res = parseConjunction();
  while( lookahead == xOr ) {
    if( res->type != "Boolean" )
      throw new std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseConjunction();
    if( r->type != "Boolean" )
      throw new std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"Or", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseConjunction()
{
  auto res = parseEquality();
  while( lookahead == xAnd ) {
    if( res->type != "Boolean" )
      throw new std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseEquality();
    if( r->type != "Boolean" )
      throw new std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"And", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseEquality()
{
  auto res = parseRelation();
  if( lookahead == xEq || lookahead == xNe ) {
    auto oper = TN[lookahead];
    lookahead = sc.next();
    res = new Binary{oper, res, parseRelation()};
  }
  return res;
}

/**/
Expression* Parser::parseRelation()
{
  auto res = parseAddition();
  if( lookahead >= xGt && lookahead <= xLe ) {
    auto oper = TN[lookahead];
    if( res->type == "Boolean" )
      throw new std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    lookahead = sc.next();
    auto r = parseAddition();
    if( res->type == "Boolean" )
      throw new std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    res = new Binary{oper, res, r};
  }
  return res;
}

/**/
Expression* Parser::parseAddition()
{
  auto res = parseMultiplication();
  while( lookahead == xAdd || lookahead == xSub ) {
    auto oper = TN[lookahead];
    lookahead = sc.next();
    res = new Binary{oper, res, parseMultiplication()};
  }
  return res;
}

/**/
Expression* Parser::parseMultiplication()
{
  auto res = parsePower();
  while( lookahead == xMul || lookahead == xDiv || lookahead == xMod ) {
    auto oper = TN[lookahead];
    lookahead = sc.next();
    res = new Binary{oper, res, parsePower()};
  }
  return res;
}

/**/
Expression* Parser::parsePower()
{
  auto res = parseFactor();
  if( lookahead == xPow ) {
    if( res->type != Expression::TyInteger || res->type != Expression::TyDouble ) 
      throw new std::logic_error{"Աստիճան կարող է բարձրացվել միայն թիվը։"};
    lookahead = sc.next();
    auto r = parsePower();
    if( res->type != Expression::TyInteger || res->type != Expression::TyDouble ) 
      throw new std::logic_error{"Աստիճանը կարող է լինել միայն թիվ։"};
    res = new Binary{"Pow", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseFactor()
{
  if( lookahead == xIdent )
    return parseVariableOrFuncCall();

  if( lookahead == xInteger ) {
    auto nm = sc.lexeme();
    match( xInteger );
    return new Constant{nm, Expression::TyInteger};
  }

  if( lookahead == xDouble ) {
    auto nm = sc.lexeme();
    match( xDouble );
    return new Constant{nm, Expression::TyDouble};
  }

  if( lookahead == xTrue ) {
    match( xTrue );
    return new Constant{"True", Expression::TyBoolean};
  }

  if( lookahead == xFalse ) {
    match( xFalse );
    return new Constant{"False", Expression::TyBoolean};
  }

  // թվային արժեքի բացասում
  if( lookahead == xSub ) {
    match( xSub );
    auto expr = parseFactor();
    if( expr->type != Expression::TyDouble && expr->type != Expression::TyInteger ) 
      throw new std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"Neg", expr};
  }

  // բուլյան արտահայտության ժխտում
  if( lookahead == xNot ) {
    match( xNot );
    auto expr = parseFactor();
    if( expr->type != Expression::TyBoolean )
      throw new std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"Not", expr};
  }

  if( lookahead == xLPar ) {
    match( xLPar );
    auto rs = parseRelation();
    match( xRPar );
    return rs;
  }
  
  std::cerr << "Syntax Error: Unexpected factor." << std::endl;
  return nullptr;
}

/**/
Expression* Parser::parseVariableOrFuncCall()
{
  auto vn = sc.lexeme();
  match( xIdent );
  auto nt = symtab->search(vn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // փոփոխականի օգտագործում
  if( lookahead != xLPar )  
    return new Variable{vn, nt.second};

  // ֆունկցիայի կանչ
  match( xLPar );
  std::vector<Expression*> es;
  auto ty = parseArguments(es);
  match( xRPar );

  return new FuncCall{vn, es};
}

