
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "parser.hxx"

namespace basic {
/* FIRST բազմությունները, վերլուծության ուղղությունն ընտրելու համար */
// հայտարարություններ ու սահմանումներ
std::set<Token> Parser::FD{ xDim, xType, xDeclare, xSubroutine, xFunction };
// ղեկավարող կառուցվածքներ
std::set<Token> Parser::FS{ xDim, xIdent, xIf, xFor, xWhile, xReturn, xInput, xPrint };
// պարզ արտահայտություններ (factor)
std::set<Token> Parser::FF{ xIdent, xInteger, xDouble, xTrue, xFalse, xSub, xNot, xLPar };

/* Այստեղից սկսվում է վերլուծությունը */
Module* Parser::parse()
{
  auto mod = new Module{file};

  // կարդալ ֆայլի սկզբի դատարկ մասը
  do
    lookahead = sc.next();
  while( lookahead.is(xEol) );

  symtab.openScope();

  try {
    while( FD.end() != FD.find(lookahead.kind) ) {
      Function* sub{nullptr};
      if( lookahead.is(xDeclare) )
	sub = parseDeclare();
      else if( lookahead.is(xFunction) )
	sub = parseFunction();
      else if( lookahead.is(xSubroutine) )
	sub = parseSubroutine();
      mod->addFunction(sub);
    }
    if( "" == symtab.search("Main").first )
      throw std::logic_error{"'Main' պրոցեդուրան սահմանված չէ։"};
  }
  catch( std::exception e ) {
    std::cerr << "ՍԽԱԼ [" << sc.line() << "]: " << e.what() << std::endl;
    delete mod;
    return nullptr;
  }
  
  symtab.closeScope();

  return mod;
}

/**/
void Parser::match( Token exp )
{
  if( !lookahead.is(exp) )
    throw std::logic_error{"ՍԽԱԼ։ Սպասվում էր `" + N(exp) + 
	"`, բայց եղել է `" + N(lookahead.kind) + "`։"};

  lookahead = sc.next();
}

/**/
void Parser::parseEols()
{
  while( lookahead.is(xEol) )
    lookahead = sc.next();
}

/**/
Statement* Parser::parseStatement()
{
  if( lookahead.is(xDim) )
    return parseDim();

  if( lookahead.is(xIdent) )
    return parseSubCallOrAssign();

  if( lookahead.is(xIf) )
    return parseIf();

  if( lookahead.is(xFor) )
    return parseFor();

  if( lookahead.is(xWhile) )
    return parseWhile();

  if( lookahead.is(xInput) )
    return parseInput();

  if( lookahead.is(xPrint) )
    return parsePrint();

  if( lookahead.is(xReturn) )
    return parseReturn();

  return nullptr;
}

/**/
Statement* Parser::parseSequence()
{
  Statement* res{nullptr};
  while( FS.end() != FS.find(lookahead.kind) )
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
  while( lookahead.is(xComma) ) {
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
  if( FF.end() != FF.find(lookahead.kind) ) {
    auto ex = parseDisjunction();
    spec << ex->type;
    es.push_back( ex );
    while( lookahead.is(xComma) ) {
      lookahead = sc.next();
      ex = parseDisjunction();
      spec << " x " << ex->type;
      es.push_back( ex );
    }
  }
  return spec.str();
}

/* TODO */
//void Parser::parseType()
//{
//  match( xType );
//  std::string nm = sc.lexeme();
//  match( xIdent );
//  match( xEol );
//  while( lookahead == xIdent ) {
//    parseNameDecl();
//    match( xEol );
//  }
//  match( xEnd );
//  match( xType );
//}

/**/
Function* Parser::parseDeclare()
{
  Function* result{nullptr};
  match( xDeclare );
  if( lookahead.is(xSubroutine) )
    result = parseSubrHeader();
  else if( lookahead.is(xFunction) )
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
  if( lookahead.is(xIdent) )
    sig = parseDeclList( ag );
  parseEols();
  if( nm == "Main" && ag.size() != 0 )
    throw std::logic_error{"'Main' պրոցեդուրան արգումենտներ չունի։"};
  symtab.insert( Symbol{nm, sig + " -> " + Expression::TyVoid} );
  return new Function{nm, ag, Expression::TyVoid};
}

/**/
Function* Parser::parseSubroutine()
{
  auto pr = parseSubrHeader();
  symtab.openScope();
  for( auto& a : pr->args )
    symtab.insert( Symbol{a.first, a.second} );
  auto bo = parseSequence();
  match( xEnd );
  match( xSubroutine );
  parseEols();
  pr->setBody( bo );
  symtab.closeScope();
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
  if( lookahead.is(xIdent) )
    sig = parseDeclList( ag );
  match( xRPar );
  match( xAs );
  std::string ty = sc.lexeme();
  match( xIdent );
  parseEols();
  symtab.insert( Symbol{nm, sig + " -> " + ty} );
  return new Function{nm, ag, ty};
}

/**/
Function* Parser::parseFunction()
{
  auto pr = parseFuncHeader();
  symtab.openScope();
  for( auto& a : pr->args )
    symtab.insert( Symbol{a.first, a.second} );
  auto bo = parseSequence();
  match( xEnd );
  match( xFunction );
  parseEols();
  pr->setBody( bo );
  symtab.closeScope();
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
  auto sy = symtab.search(nv.first);
  if( "" != sy.first ) 
    throw new std::logic_error{"'" + nv.first + "' անունն արդեն հայտարարված է։"};
  parseEols();
  symtab.insert(nv);
  return new Declare{nv.first, nv.second};
}

/**/
Statement* Parser::parseSubCallOrAssign()
{
  // փոփոխականի կամ պրոցեդուրայի անուն
  auto vn = sc.lexeme();
  match( xIdent );
  auto nt = symtab.search(vn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // վերագրման հրաման
  if( lookahead.is(xEq) ) {
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
  while( lookahead.is(xElseIf) ) {
    match( xElseIf );
    cond = parseDisjunction();
    match( xThen );
    parseEols();
    thenp = parseSequence();
    auto elsep = new Branch( cond, thenp, nullptr );
    brit->setElse( elsep );
    brit = elsep;
  }
  if( lookahead.is(xElse) ) {
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
  auto nt = symtab.search(cn);
  if( "" == nt.first )
    throw std::logic_error{"Չհայտարարված անուն '" + cn +"'։"};
  if( Expression::TyInteger != nt.second )
    throw std::logic_error{"'For' ցիկլի հաշվիչի տիպը պետք է լինի Integer։"};
  match( xEq );
  auto st = parseAddition(); // ? type is Integer
  match( xTo );
  auto ed = parseAddition(); // ? type is Integer
  Expression* sp{nullptr};
  if( lookahead.is(xStep) ) {
    match( xStep );
    sp = parseAddition(); // ? type is Integer
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
  symbolvector vars;
  auto nm = sc.lexeme();
  match( xIdent );
  auto sy = symtab.search(nm);
  if( "" == sy.first )
    throw std::logic_error{"Չհայտարարված անուն '" + nm +"'։"};
  vars.push_back( sy );
  while( lookahead.is(xComma) ) {
    lookahead = sc.next();
    nm = sc.lexeme();
    match( xIdent );
    sy = symtab.search(nm);
    if( "" == sy.first )
      throw std::logic_error{"Չհայտարարված անուն '" + nm +"'։"};
    vars.push_back( sy );
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
  while( lookahead.is(xComma) ) {
    lookahead = sc.next();
    ex = parseDisjunction();
    vals.push_back( ex );
  }
  parseEols();
  return new Print{vals};  /**/

}

/**/
Expression* Parser::parseDisjunction()
{
  auto res = parseConjunction();
  while( lookahead.is(xOr) ) {
    if( res->type != Expression::TyBoolean )
      throw std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseConjunction();
    if( r->type != Expression::TyBoolean )
      throw std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"or", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseConjunction()
{
  auto res = parseEquality();
  while( lookahead.is(xAnd) ) {
    if( res->type != Expression::TyBoolean )
      throw std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseEquality();
    if( r->type != Expression::TyBoolean )
      throw std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"and", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseEquality()
{
  auto res = parseRelation();
  if( lookahead.is(xEq) || lookahead.is(xNe) ) {
    auto oper = N(lookahead.kind);
    lookahead = sc.next();
    res = new Binary{oper, res, parseRelation()};
  }
  return res;
}

/**/
Expression* Parser::parseRelation()
{
  auto res = parseAddition();
  if( lookahead.in(xGt, xLe) ) {
    auto oper = N(lookahead.kind);
    if( res->type == Expression::TyBoolean )
      throw std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    lookahead = sc.next();
    auto r = parseAddition();
    if( res->type == Expression::TyBoolean )
      throw std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    res = new Binary{oper, res, r};
  }
  return res;
}

/**/
Expression* Parser::parseAddition()
{
  auto res = parseMultiplication();
  while( lookahead.is(xAdd) || lookahead.is(xSub) ) {
    auto oper = N(lookahead.kind);
    lookahead = sc.next();
    res = new Binary{oper, res, parseMultiplication()};
  }
  return res;
}

/**/
Expression* Parser::parseMultiplication()
{
  auto res = parsePower();
  while( lookahead.is(xMul) || lookahead.is(xDiv) || lookahead.is(xMod) ) {
    auto oper = N(lookahead.kind);
    lookahead = sc.next();
    res = new Binary{oper, res, parsePower()};
  }
  return res;
}

/**/
Expression* Parser::parsePower()
{
  auto res = parseFactor();
  if( lookahead.is(xPow) ) {
    if( res->type != Expression::TyInteger && res->type != Expression::TyDouble ) 
      throw std::logic_error{"Աստիճան կարող է բարձրացվել միայն թիվը։"};
    lookahead = sc.next();
    auto r = parsePower();
    if( res->type != Expression::TyInteger && res->type != Expression::TyDouble ) 
      throw std::logic_error{"Աստիճանը կարող է լինել միայն թիվ։"};
    res = new Binary{"^", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseFactor()
{
  if( lookahead.is(xIdent) )
    return parseVariableOrFuncCall();

  if( lookahead.is(xInteger) ) {
    auto nm = sc.lexeme();
    match( xInteger );
    return new Constant{nm, Expression::TyInteger};
  }

  if( lookahead.is(xDouble) ) {
    auto nm = sc.lexeme();
    match( xDouble );
    return new Constant{nm, Expression::TyDouble};
  }

  if( lookahead.is(xTrue) ) {
    match( xTrue );
    return new Constant{"true", Expression::TyBoolean};
  }

  if( lookahead.is(xFalse) ) {
    match( xFalse );
    return new Constant{"false", Expression::TyBoolean};
  }

  // թվային արժեքի բացասում
  if( lookahead.is(xSub) ) {
    match( xSub );
    auto expr = parseFactor();
    if( expr->type != Expression::TyDouble && expr->type != Expression::TyInteger ) 
      throw std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"neg", expr};
  }

  // բուլյան արտահայտության ժխտում
  if( lookahead.is(xNot) ) {
    match( xNot );
    auto expr = parseFactor();
    if( expr->type != Expression::TyBoolean )
      throw std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"not", expr};
  }

  if( lookahead.is(xLPar) ) {
    match( xLPar );
    auto rs = parseRelation();
    match( xRPar );
    return rs;
  }
  
  throw std::logic_error{"Չսպասված արտադրիչ (factor)։"};
}

/**/
Expression* Parser::parseVariableOrFuncCall()
{
  auto vn = sc.lexeme();
  match( xIdent );
  auto nt = symtab.search(vn);
  if( "" == nt.first )
    throw std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // փոփոխականի օգտագործում
  if( !lookahead.is(xLPar) )  
    return new Variable{vn, nt.second};

  // ֆունկցիայի կանչ
  match( xLPar );
  std::vector<Expression*> es;
  auto ty = parseArguments(es);
  match( xRPar );

  return new FuncCall{vn, es};
}

} // basic

