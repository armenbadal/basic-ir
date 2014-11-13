
#include <iostream>

#include "parser.hxx"

namespace {
  std::vector<std::string> TN = {
    "NIL", 
    "EOL", 
    "Integer",
    "Real",
    "True",
    "False",
    "Ident",
    "Dim",
    "As",
    "Type",
    "End",
    "Declare",
    "Sub",
    "Function",
    "Let",
    "If",
    "Then",
    "ElseIf",
    "Else",
    "For",
    "To",
    "Step",
    "While",
    "(",
    ")",
    ",",
    "=",
    "<>",
    ">",
    ">=",
    "<",
    "<=",
    "+",
    "-",
    "*",
    "/",
    "\\",
    "^",
    "EOF"
  };
}

/**/
Parser::Parser( const char* name )
  : sc{name}
{
  do
    lookahead = sc.next();
  while( lookahead == xEol );
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

/**/
std::set<Token> Parser::FD{ xDim, xType, xDeclare, xSubroutine, xFunction };
std::set<Token> Parser::FS{ xDim, xLet, xIf, xFor, xWhile };

/**/
bool Parser::inSet( const std::set<Token>& es )
{
  return es.end() != es.find(lookahead);
}

/**/
void Parser::parse()
{
  while( inSet(FD) ) {
    parseGlobal();
  }
  /* DEBUG */ std::cout << "PARSED" << std::endl;
}

/**/
void Parser::parseGlobal()
{
  switch( lookahead ) {
    case xDim:
      parseDim();
      break;
    case xType:
      parseType();
      break;
    case xDeclare:
      parseDeclare();
      break;
    case xSubroutine:
      parseSubroutine();
      break;
    case xFunction:
      parseFunction();
      break;
    default:
      break;
  }
}

/**/
void Parser::parseStatement()
{
  switch( lookahead ) {
    case xDim:
      parseDim();
      break;
    case xLet:
      parseLet();
      break;
    case xIf:
      parseIf();
      break;
    case xFor:
      parseFor();
      break;
    case xWhile:
      parseWhile();
      break;
    default:
      break;
  }
}

/**/
void Parser::parseSequence()
{
  while( inSet(FS) )
    parseStatement();
}

/**/
void Parser::parseDim()
{
  match( xDim );
  auto nv = parseNameDecl();
  parseEols();
  /* DEBUG */ std::cout << '<' << nv.first << ',' << nv.second << '>' << std::endl;
}

/**/
std::pair<std::string,std::string> Parser::parseNameDecl()
{
  std::string nm = sc.lexeme();
  match( xIdent );
  match( xAs );
  std::string ty = sc.lexeme();
  match( xIdent );
  return std::make_pair(nm,ty);
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
void Parser::parseDeclare()
{
  match( xDeclare );
  if( lookahead == xSubroutine )
    parseSubrHeader();
  else if( lookahead == xFunction )
    parseFuncHeader();
}

/**/
void Parser::parseSubrHeader()
{
  match( xSubroutine );
  std::string nm = sc.lexeme();
  match( xIdent );
  if( lookahead == xLPar ) {
    match( xLPar );
    match( xRPar );
  }
  parseEols();
}

/**/
void Parser::parseSubroutine()
{
  parseSubrHeader();
  parseSequence();
  match( xEnd );
  match( xSubroutine );
  parseEols();
}

/**/
void Parser::parseFuncHeader()
{
  match( xFunction );
  match( xIdent );
  match( xLPar );
  match( xRPar );
  match( xAs );
  match( xIdent );
  match( xEol );
}

/**/
void Parser::parseFunction()
{
  parseFuncHeader();
  // parse statement list
  match( xEnd );
  match( xFunction );
  match( xEol );
}

/**/
void Parser::parseLet()
{
  match( xLet );
  auto vn = sc.lexeme();
  match( xIdent );
  match( xEq );
  parseRelation();
  parseEols();
  /* DEBUG */ std::cout << vn << std::endl;
}

/**/
void Parser::parseIf()
{
  match( xIf );
  parseRelation();
  match( xThen );
  parseEols();
  parseSequence();
  while( lookahead == xElseIf ) {
    match( xElseIf );
    parseRelation();
    match( xThen );
    parseEols();
    parseSequence();
  }
  if( lookahead == xElse ) {
    match( xElse );
    parseEols();
    parseSequence();
  }
  match( xEnd );
  match( xIf );
  parseEols();
}

/**/
void Parser::parseFor()
{
  match( xFor );
  match( xIdent );
  match( xEq );
  parseExpression();
  match( xTo );
  parseExpression();
  if( lookahead == xStep ) {
    match( xStep );
    parseExpression();
  }
  parseEols();
  parseSequence();
  match( xEnd );
  match( xFor );
  parseEols();
}

/**/
void Parser::parseWhile()
{
  match( xWhile );
  parseRelation();
  parseEols();
  parseSequence();
  match( xEnd );
  match( xWhile );
  parseEols();
}

/**/
void Parser::parseRelation()
{
  parseExpression();
  if( lookahead >= xEq && lookahead <= xLe ) {
    lookahead = sc.next();
    parseExpression();
  }
}

/**/
void Parser::parseExpression()
{
  parseTerm();
  while( lookahead == xAdd || lookahead == xSub ) {
    lookahead = sc.next();
    parseTerm();
  }
}

/**/
void Parser::parseTerm()
{
  parseFactor();
  while( lookahead == xMul || lookahead == xDiv || lookahead == xMod ) {
    lookahead = sc.next();
    parseFactor();
  }
}

/**/
void Parser::parseFactor()
{
  if( lookahead == xIdent )
    match( xIdent );
  else if( lookahead == xInteger )
    match( xInteger );
  else if( lookahead == xReal )
    match( xReal );
  else if( lookahead == xTrue )
    match( xTrue );
  else if( lookahead == xFalse )
    match( xFalse );
  else if( lookahead == xSub ) {
    match( xSub );
    parseFactor();
  }
  else if( lookahead == xLPar ) {
    match( xLPar );
    parseRelation();
    match( xRPar );
  }
  else
    std::cerr << "Syntax Error: Unexpected factor" << std::endl;
}

