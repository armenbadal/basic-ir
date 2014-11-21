
#include <iostream>
#include <sstream>

#include "parser.hxx"

namespace {
  /**/
  std::vector<std::string> TN = {
    "NIL", "EOL", "Integer", "Double", "True", "False", 
    "Ident", "Dim", "As", "Type", "End", "Declare", "Sub", 
    "Function", "Return", "Let", "If", "Then", "ElseIf",
    "Else", "For", "To", "Step", "While", "Input", "Print",
    "(", ")", ",", "And", "Or", "Not", "=", "<>", ">", 
    ">=", "<", "<=", "+", "-", "*", "/", "\\", "^", "EOF"
  };

  /**/
  template<typename T>
  T asNumber(const std::string& v)
  {
    std::stringstream ss(v);
    T num{0};
    ss >> num;
    return num;
  }
}

/**/
void Parser::match( Token exp )
{
  if( lookahead == exp )
    lookahead = sc.next();
  else
    std::cerr << "Syntax error: Expected `" 
	      << TN[exp] << "' but got `" 
	      << TN[lookahead] << "'." 
	      << std::endl;
}

/**/
void Parser::parseEols()
{
  while( lookahead == xEol )
    lookahead = sc.next();
}

/* FIRST sets */
std::set<Token> Parser::FD{ xDim, xType, xDeclare, xSubroutine, xFunction };
std::set<Token> Parser::FS{ xDim, xLet, xIdent, xIf, xFor, xWhile, xReturn };
std::set<Token> Parser::FF{ xIdent, xInteger, xDouble, xTrue, xFalse, xSub, xLPar };

/**/
bool Parser::inSet( const std::set<Token>& es )
{
  return es.end() != es.find(lookahead);
}

/* Այստեղից սկսվում է վերլուծությունը */
Module* Parser::parse()
{
  Module* mod = new Module(file);

  do
    lookahead = sc.next();
  while( lookahead == xEol );

  symtab->openScope(); // ?

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
  /* DEBUG */ std::cout << "PARSED" << std::endl;
  return mod;
}

/**/
Statement* Parser::parseStatement()
{
  if( lookahead == xDim )
    return parseDim();

  if( lookahead == xLet )
    return parseLet();

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
      res = new Sequence(res, parseStatement());
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
  return std::make_pair( a0, a1 );
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
  match( xDeclare );
  if( lookahead == xSubroutine )
    return parseSubrHeader();
  if( lookahead == xFunction )
    return parseFuncHeader();
  return nullptr;
}

/**/
Function* Parser::parseSubrHeader()
{
  match( xSubroutine );
  std::string nm = sc.lexeme();
  match( xIdent );
  std::string sig{"()"};
  std::vector<Symbol> ag;
  if( lookahead == xLPar ) {
    match( xLPar );
    if( lookahead == xIdent )
      sig = parseDeclList( ag );
    match( xRPar );
  }
  parseEols();
  
  symtab->insert( Symbol(nm, sig + " -> Void") );

  return new Function(nm, ag, "Void");
}

/**/
Function* Parser::parseSubroutine()
{
  auto pr = parseSubrHeader();
  symtab->openScope();
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

  symtab->insert( Symbol(nm, sig + " -> " + ty) );

  return new Function(nm, ag, ty);
}

/**/
Function* Parser::parseFunction()
{
  auto pr = parseFuncHeader();
  symtab->openScope();
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
  return new Result(rv);
}

/**/
Statement* Parser::parseDim()
{
  match( xDim );
  auto nv = parseNameDecl();
  parseEols();
  symtab->insert(nv);
  return new Declare(nv.first, nv.second);
}

/**/
Statement* Parser::parseAssignment()
{
  auto vn = sc.lexeme();
  match( xIdent );
  match( xEq );
  auto ex = parseDisjunction();
  parseEols();
  return new Assign(vn, ex);
}

/**/
Statement* Parser::parseLet()
{
  match( xLet );
  return parseAssignment();
}

/**/
Statement* Parser::parseSubCall()
{
  auto vn = sc.lexeme();
  match( xIdent );
  if( lookahead == xLPar ) {
    match( xLPar );
    match( xRPar );
  }
  parseEols();
  return nullptr;
}

/**/
Statement* Parser::parseIf()
{
  match( xIf );
  auto cond = parseDisjunction();
  match( xThen );
  parseEols();
  auto thenp = parseSequence();
  auto branch = new Branch( cond, thenp, nullptr );
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
  match( xEq );
  auto st = parseAddition(); // !
  match( xTo );
  auto ed = parseAddition(); // !
  Expression* sp{nullptr};
  if( lookahead == xStep ) {
    match( xStep );
    sp = parseAddition(); // !
  }
  parseEols();
  auto body = parseSequence();
  match( xEnd );
  match( xFor );
  parseEols();
  return nullptr;
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
  return new WhileLoop(cond, body);
}

/**/
Statement* Parser::parseInput()
{
  match( xInput );
  match( xIdent );
  while( lookahead == xComma ) {
    lookahead = sc.next();
    match( xIdent );
  }
  parseEols();
  return nullptr;
}

/**/
Statement* Parser::parsePrint()
{
  match( xPrint );
  parseDisjunction();
  while( lookahead == xComma ) {
    lookahead = sc.next();
    parseDisjunction();
  }
  parseEols();
  return nullptr;
}

/**/
Expression* Parser::parseDisjunction()
{
  auto res = parseConjunction();
  while( lookahead == xOr ) {
    lookahead = sc.next();
    res = new Binary("Or", res, parseConjunction());
  }
  return res;
}

/**/
Expression* Parser::parseConjunction()
{
  auto res = parseEquality();
  while( lookahead == xAnd ) {
    lookahead = sc.next();
    res = new Binary("And", res, parseEquality());
  }
  return res;
}

/**/
Expression* Parser::parseEquality()
{
  auto res = parseRelation();
  if( lookahead == xEq || lookahead == xNe ) {
    lookahead = sc.next();
    res = new Binary("?", res, parseRelation());
  }
  return res;
}

/**/
Expression* Parser::parseRelation()
{
  auto res = parseAddition();
  if( lookahead >= xGt && lookahead <= xLe ) {
    lookahead = sc.next();
    res = new Binary("?", res, parseAddition());
  }
  return res;
}

/**/
Expression* Parser::parseAddition()
{
  auto res = parseMultiplication();
  while( lookahead == xAdd || lookahead == xSub ) {
    lookahead = sc.next();
    res = new Binary("?", res, parseMultiplication());
  }
  return res;
}

/**/
Expression* Parser::parseMultiplication()
{
  auto res = parsePower();
  while( lookahead == xMul || lookahead == xDiv || lookahead == xMod ) {
    lookahead = sc.next();
    res = new Binary("?", res, parsePower());
  }
  return res;
}

/**/
Expression* Parser::parsePower()
{
  auto res = parseFactor();
  if( lookahead == xPow ) {
    lookahead = sc.next();
    res = new Binary("Pow", res, parsePower());
  }
  return res;
}

/**/
Expression* Parser::parseFactor()
{
  if( lookahead == xIdent )
    return parseVariableOrFuncCall();

  if( lookahead == xInteger ) {
    auto nm = asNumber<int>(sc.lexeme());
    match( xInteger );
    return new Integer(nm);
  }

  if( lookahead == xDouble ) {
    auto nm = asNumber<double>(sc.lexeme());
    match( xDouble );
    return new Double(nm);
  }

  if( lookahead == xTrue ) {
    match( xTrue );
    return new Boolean(true);
  }

  if( lookahead == xFalse ) {
    match( xFalse );
    return new Boolean(false);
  }

  // թվային արժեքի բացասում
  if( lookahead == xSub ) {
    match( xSub );
    auto expr = parseFactor();
    auto y = expr->type;
    if( y != "Double" && y != "Integer" ) {
      std::cerr << "Սխալ։ Անհամապատասխան տիպեր։" << std::endl;
      return nullptr;
    }
    std::string op{"Neg"};
    if( y == "Double" ) op = "FNeg";
    expr = new Unary{op, expr};
    expr->type = y;
    return expr;
  }

  // բուլյան արտահայտության ժխտում
  if( lookahead == xNot ) {
    match( xNot );
    auto expr = parseFactor();
    if( expr->type != "Boolean" ) {
      std::cerr << "Սխալ։ Անհամապատասխան տիպեր։" << std::endl;
      return nullptr;
    }
    expr = new Unary{"Not", expr};
    expr->type = "Boolean";
    return expr;
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
  if( "" == nt.first ) {
    std::cerr << "Սխալ։ Չհայտարարված անուն։" << std::endl;
    return nullptr;
  }

  // փոփոխականի օգտագործում
  if( lookahead != xLPar )  
    return new Variable{vn, nt.second};
///* DEBUG */ std::cout << "HERE" << std::endl;
  // ֆունկցիայի կանչ
  std::stringstream ss;
  std::vector<Expression*> ps;
  match( xLPar );
  auto ex = parseRelation();
  ss << ex->type;
  ps.push_back( ex );
  while( lookahead == xComma ) {
    lookahead = sc.next();
    ex = parseRelation();
    ss << " x " << ex->type;
    ps.push_back( ex );
  }
  match( xRPar );
  // TODO ստուգել արգումենտների և պարամետրերի համապատասխանությունը
///* DEBUG */ std::cout << nt.first << ":" << nt.second << std::endl;
///* DEBUG */ std::cout << ss.str() << std::endl;
  return new FuncCall( vn, ps );
}

