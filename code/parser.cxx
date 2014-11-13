
#include <iostream>
#include <string>

#include "parser.hxx"

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
    std::cerr << "Syntax error." << std::endl;
}

/**/
void Parser::parseEol()
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
    /* DEBUG */ std::cout << sc.line() << ' ';
    /* DEBUG */ std::cout << lookahead << std::endl;
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
  std::string nm = sc.lexeme();
  parseNameDecl();
  match( xEol );
}

/**/
void Parser::parseNameDecl()
{
  match( xIdent );
  match( xAs );
  match( xIdent );
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
  std::tuple<std::string,std::pair<std::string,std::string>>> Parser::parseSubrHeader()
{
  match( xSubroutine );
  std::string nm = sc.lexeme();
  match( xIdent );
  if( lookahead == xLPar ) {
    match( xLPar );
    match( xRPar );
  }
  parseEol();
}

/**/
void Parser::parseSubroutine()
{
  parseSubrHeader();
  parseSequence();
  match( xEnd );
  match( xSubroutine );
  parseEol();
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
}

/**/
void Parser::parseIf()
{
}

/**/
void Parser::parseFor()
{
}

/**/
void Parser::parseWhile()
{
}

