
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "parser.hxx"

/* Թոքենների անունները, հաղորդագրություններում օգտագործելու համար */
std::vector<std::string> Parser::TN = {
  "NIL", "EOL", "Integer", "Double", "True", "False", 
  "Ident", "Dim", "As", "Type", "End", "Declare", "Sub", 
  "Function", "Return", "If", "Then", "ElseIf", "Else",
  "For", "To", "Step", "While", "Input", "Print",
  "(", ")", ",", "And", "Or", "Not", "=", "<>", ">", 
  ">=", "<", "<=", "+", "-", "*", "/", "\\", "^", "EOF"
};

/* FIRST բազմությունները, վերլուծության ուղղությունն ընտրելու համար */
// հայտարարություններ ու սահմանումներ
std::set<Token> Parser::FD{ Token::xDim, Token::xType, Token::xDeclare, Token::xSubroutine, Token::xFunction };
// ղեկավարող կառուցվածքներ
std::set<Token> Parser::FS{ Token::xDim, Token::xIdent, Token::xIf, Token::xFor, Token::xWhile, Token::xReturn, Token::xInput, Token::xPrint };
// պարզ արտահայտություններ (factor)
std::set<Token> Parser::FF{ Token::xIdent, Token::xInteger, Token::xDouble, Token::xTrue, Token::xFalse, Token::xSub, Token::xNot, Token::xLPar };

/* Այստեղից սկսվում է վերլուծությունը */
Module* Parser::parse()
{
  Module* mod = new Module{file};

  do
    lookahead = sc.next();
  while( lookahead == Token::xEol );

  symtab.openScope();

  try {
    while( FD.end() != FD.find(lookahead) ) {
      Function* sub{nullptr};
      if( lookahead == Token::xDeclare )
	sub = parseDeclare();
      else if( lookahead == Token::xFunction )
	sub = parseFunction();
      else if( lookahead == Token::xSubroutine )
	sub = parseSubroutine();
      mod->addFunction(sub);
    }
    if( "" == symtab.search("Main").first )
      throw new std::logic_error{"'Main' պրոցեդուրան սահմանված չէ։"};
  }
  catch( std::exception* e ) {
    std::cerr << "ՍԽԱԼ [" << sc.line() << "]: " << e->what() << std::endl;
    delete e; delete mod;
    return nullptr;
  }
  
  symtab.closeScope();

  return mod;
}

/**/
void Parser::match( Token exp )
{
  if( lookahead == exp )
    lookahead = sc.next();
  else {
    auto e = static_cast<int>(exp);
    auto l = static_cast<int>(lookahead);
    throw new std::logic_error{"ՍԽԱԼ։ Սպասվում էր `" + TN[e] + 
	"', բայց եղել է `" + TN[l] + "'։"};
  }
}

/**/
void Parser::parseEols()
{
  while( lookahead == Token::xEol )
    lookahead = sc.next();
}

/**/
Statement* Parser::parseStatement()
{
  if( lookahead == Token::xDim )
    return parseDim();

  if( lookahead == Token::xIdent )
    return parseSubCallOrAssign();

  if( lookahead == Token::xIf )
    return parseIf();

  if( lookahead == Token::xFor )
    return parseFor();

  if( lookahead == Token::xWhile )
    return parseWhile();

  if( lookahead == Token::xInput )
    return parseInput();

  if( lookahead == Token::xPrint )
    return parsePrint();

  if( lookahead == Token::xReturn )
    return parseReturn();

  return nullptr;
}

/**/
Statement* Parser::parseSequence()
{
  Statement* res{nullptr};
  while( FS.end() != FS.find(lookahead) )
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
  match( Token::xIdent );
  match( Token::xAs );
  auto a1 = sc.lexeme();
  match( Token::xIdent );
  return std::make_pair(a0, a1);
}

/**/
std::string Parser::parseDeclList(std::vector<Symbol>& ds)
{
  std::stringstream ss;
  auto nv = parseNameDecl();
  ds.push_back( nv );
  ss << nv.second;
  while( lookahead == Token::xComma ) {
    match( Token::xComma );
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
  if( FF.end() != FF.find(lookahead) ) {
    auto ex = parseDisjunction();
    spec << ex->type;
    es.push_back( ex );
    while( lookahead == Token::xComma ) {
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
  match( Token::xDeclare );
  if( lookahead == Token::xSubroutine )
    result = parseSubrHeader();
  else if( lookahead == Token::xFunction )
    result = parseFuncHeader();
  return result;
}

/**/
Function* Parser::parseSubrHeader()
{
  match( Token::xSubroutine );
  std::string nm = sc.lexeme();
  match( Token::xIdent );
  std::string sig{"()"};
  std::vector<Symbol> ag;
  if( lookahead == Token::xIdent )
    sig = parseDeclList( ag );
  parseEols();
  if( nm == "Main" && ag.size() != 0 )
    throw new std::logic_error{"'Main' պրոցեդուրան արգումենտներ չունի։"};
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
  match( Token::xEnd );
  match( Token::xSubroutine );
  parseEols();
  pr->setBody( bo );
  symtab.closeScope();
  return pr;
}

/**/
Function* Parser::parseFuncHeader()
{
  match( Token::xFunction );
  std::string nm = sc.lexeme();
  match( Token::xIdent );
  if( nm == "Main" )
    throw new std::logic_error{"'Main'-ը պետք է լինի պրոցեդուրա։"};
  match( Token::xLPar );
  std::string sig{"()"};
  std::vector<Symbol> ag;
  if( lookahead == Token::xIdent )
    sig = parseDeclList( ag );
  match( Token::xRPar );
  match( Token::xAs );
  std::string ty = sc.lexeme();
  match( Token::xIdent );
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
  match( Token::xEnd );
  match( Token::xFunction );
  parseEols();
  pr->setBody( bo );
  symtab.closeScope();
  return pr;
}

/**/
Statement* Parser::parseReturn()
{
  match( Token::xReturn );
  auto rv = parseDisjunction();
  parseEols();
  return new Result{rv};
}

/**/
Statement* Parser::parseDim()
{
  match( Token::xDim );
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
  match( Token::xIdent );
  auto nt = symtab.search(vn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // վերագրման հրաման
  if( lookahead == Token::xEq ) {
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
  match( Token::xIf );
  auto cond = parseDisjunction();
  match( Token::xThen );
  parseEols();
  auto thenp = parseSequence();
  auto branch = new Branch{cond, thenp, nullptr};
  auto brit = branch;
  while( lookahead == Token::xElseIf ) {
    match( Token::xElseIf );
    cond = parseDisjunction();
    match( Token::xThen );
    parseEols();
    thenp = parseSequence();
    auto elsep = new Branch( cond, thenp, nullptr );
    brit->setElse( elsep );
    brit = elsep;
  }
  if( lookahead == Token::xElse ) {
    match( Token::xElse );
    parseEols();
    auto elsep = parseSequence();
    brit->setElse( elsep );
  }
  match( Token::xEnd );
  match( Token::xIf );
  parseEols();
  return branch;
}

/**/
Statement* Parser::parseFor()
{
  match( Token::xFor );
  auto cn = sc.lexeme();
  match( Token::xIdent );
  auto nt = symtab.search(cn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + cn +"'։"};
  if( Expression::TyInteger != nt.second )
    throw new std::logic_error{"'For' ցիկլի հաշվիչի տիպը պետք է լինի Integer։"};
  match( Token::xEq );
  auto st = parseAddition(); // ? type is Integer
  match( Token::xTo );
  auto ed = parseAddition(); // ? type is Integer
  Expression* sp{nullptr};
  if( lookahead == Token::xStep ) {
    match( Token::xStep );
    sp = parseAddition(); // ? type is Integer
  }
  parseEols();
  auto bo = parseSequence();
  match( Token::xEnd );
  match( Token::xFor );
  parseEols();
  return new ForLoop{cn, st, ed, sp, bo};
}

/**/
Statement* Parser::parseWhile()
{
  match( Token::xWhile );
  auto cond = parseDisjunction();
  parseEols();
  auto body = parseSequence();
  match( Token::xEnd );
  match( Token::xWhile );
  parseEols();
  return new WhileLoop{cond, body};
}

/**/
Statement* Parser::parseInput()
{
  match( Token::xInput );
  symbolvector vars;
  auto nm = sc.lexeme();
  match( Token::xIdent );
  auto sy = symtab.search(nm);
  if( "" == sy.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + nm +"'։"};
  vars.push_back( sy );
  while( lookahead == Token::xComma ) {
    lookahead = sc.next();
    nm = sc.lexeme();
    match( Token::xIdent );
    sy = symtab.search(nm);
    if( "" == sy.first )
      throw new std::logic_error{"Չհայտարարված անուն '" + nm +"'։"};
    vars.push_back( sy );
  }
  parseEols();
  return new Input{vars};
}

/**/
Statement* Parser::parsePrint()
{
  match( Token::xPrint );
  std::vector<Expression*> vals;
  auto ex =  parseDisjunction();
  vals.push_back( ex );
  while( lookahead == Token::xComma ) {
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
  while( lookahead == Token::xOr ) {
    if( res->type != Expression::TyBoolean )
      throw new std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseConjunction();
    if( r->type != Expression::TyBoolean )
      throw new std::logic_error{"'Or' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"or", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseConjunction()
{
  auto res = parseEquality();
  while( lookahead == Token::xAnd ) {
    if( res->type != Expression::TyBoolean )
      throw new std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    lookahead = sc.next();
    auto r = parseEquality();
    if( r->type != Expression::TyBoolean )
      throw new std::logic_error{"'And' գործողությունը սպասում է բուլյան արգումենտ։"};
    res = new Binary{"and", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseEquality()
{
  auto res = parseRelation();
  if( lookahead == Token::xEq || lookahead == Token::xNe ) {
    auto oper = TN[static_cast<int>(lookahead)];
    lookahead = sc.next();
    res = new Binary{oper, res, parseRelation()};
  }
  return res;
}

/**/
Expression* Parser::parseRelation()
{
  auto res = parseAddition();
  if( lookahead >= Token::xGt && lookahead <= Token::xLe ) {
    auto oper = TN[static_cast<int>(lookahead)];
    if( res->type == Expression::TyBoolean )
      throw new std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    lookahead = sc.next();
    auto r = parseAddition();
    if( res->type == Expression::TyBoolean )
      throw new std::logic_error{"'" + oper + "' գործողության արգումենտը չի կարող լինել բուլյան։"};
    res = new Binary{oper, res, r};
  }
  return res;
}

/**/
Expression* Parser::parseAddition()
{
  auto res = parseMultiplication();
  while( lookahead == Token::xAdd || lookahead == Token::xSub ) {
    auto oper = TN[static_cast<int>(lookahead)];
    lookahead = sc.next();
    res = new Binary{oper, res, parseMultiplication()};
  }
  return res;
}

/**/
Expression* Parser::parseMultiplication()
{
  auto res = parsePower();
  while( lookahead == Token::xMul || lookahead == Token::xDiv || lookahead == Token::xMod ) {
    auto oper = TN[static_cast<int>(lookahead)];
    lookahead = sc.next();
    res = new Binary{oper, res, parsePower()};
  }
  return res;
}

/**/
Expression* Parser::parsePower()
{
  auto res = parseFactor();
  if( lookahead == Token::xPow ) {
    if( res->type != Expression::TyInteger && res->type != Expression::TyDouble ) 
      throw new std::logic_error{"Աստիճան կարող է բարձրացվել միայն թիվը։"};
    lookahead = sc.next();
    auto r = parsePower();
    if( res->type != Expression::TyInteger && res->type != Expression::TyDouble ) 
      throw new std::logic_error{"Աստիճանը կարող է լինել միայն թիվ։"};
    res = new Binary{"^", res, r};
  }
  return res;
}

/**/
Expression* Parser::parseFactor()
{
  if( lookahead == Token::xIdent )
    return parseVariableOrFuncCall();

  if( lookahead == Token::xInteger ) {
    auto nm = sc.lexeme();
    match( Token::xInteger );
    return new Constant{nm, Expression::TyInteger};
  }

  if( lookahead == Token::xDouble ) {
    auto nm = sc.lexeme();
    match( Token::xDouble );
    return new Constant{nm, Expression::TyDouble};
  }

  if( lookahead == Token::xTrue ) {
    match( Token::xTrue );
    return new Constant{"true", Expression::TyBoolean};
  }

  if( lookahead == Token::xFalse ) {
    match( Token::xFalse );
    return new Constant{"false", Expression::TyBoolean};
  }

  // թվային արժեքի բացասում
  if( lookahead == Token::xSub ) {
    match( Token::xSub );
    auto expr = parseFactor();
    if( expr->type != Expression::TyDouble && expr->type != Expression::TyInteger ) 
      throw new std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"neg", expr};
  }

  // բուլյան արտահայտության ժխտում
  if( lookahead == Token::xNot ) {
    match( Token::xNot );
    auto expr = parseFactor();
    if( expr->type != Expression::TyBoolean )
      throw new std::logic_error{"Անհամապատասխան տիպեր։"};
    return new Unary{"not", expr};
  }

  if( lookahead == Token::xLPar ) {
    match( Token::xLPar );
    auto rs = parseRelation();
    match( Token::xRPar );
    return rs;
  }
  
  std::cerr << "Syntax Error: Unexpected factor." << std::endl;
  return nullptr;
}

/**/
Expression* Parser::parseVariableOrFuncCall()
{
  auto vn = sc.lexeme();
  match( Token::xIdent );
  auto nt = symtab.search(vn);
  if( "" == nt.first )
    throw new std::logic_error{"Չհայտարարված անուն '" + vn +"'։"};

  // փոփոխականի օգտագործում
  if( lookahead != Token::xLPar )  
    return new Variable{vn, nt.second};

  // ֆունկցիայի կանչ
  match( Token::xLPar );
  std::vector<Expression*> es;
  auto ty = parseArguments(es);
  match( Token::xRPar );

  return new FuncCall{vn, es};
}

