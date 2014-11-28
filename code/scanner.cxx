
#include <cctype>
#include <sstream>

#include "scanner.hxx"

#include <iostream>


std::map<std::string,Token> Scanner::keywords{
  {"Dim",xDim},
  {"As", xAs},
  {"Type", xType},
  {"True", xTrue},
  {"False", xFalse},
  {"End", xEnd},
  {"Sub", xSubroutine},
  {"Function", xFunction},
  {"Return", xReturn},
  {"Let", xLet},
  {"If", xIf},
  {"Then", xThen},
  {"ElseIf", xElseIf},
  {"Else", xElse},
  {"For", xFor},
  {"To", xTo},
  {"Step", xStep},
  {"While", xWhile},
  {"Input", xInput},
  {"Print", xPrint},
  {"And", xAnd},
  {"Or", xOr},
  {"Not", xNot}
};

/**/
Scanner::Scanner( const std::string& name )
  : source{name.c_str()}
{
  source.unsetf( std::ios::skipws );
  source >> c;
}

/**/
std::string Scanner::lexeme() const
{ return text; }

/**/
Token Scanner::next()
{
  text = "";

  // whitespaces
  while( c == ' ' || c == '\t' ) source >> c;

  // end of file
  if( c == -1 || source.eof() ) return xEof;

  // comments
  if( c == '\'' ) {
    while( c != '\n' ) source >> c;
    return next();
  }

  // newline
  if( c == '\n' ) {
    ++linenum;
    source >> c;
    return xEol;
  }

  // lexeme 
  text = "";

  // integers and reals
  if( isdigit(c) ) {
    text = sequence( isdigit );
    if( c != '.' ) return xInteger;
    text += "."; source >> c;
    text += sequence( isdigit );
    return xDouble;
  }

  // identifiers and keywords
  if( isalpha( c ) ) {
    text = sequence( isalnum );
    std::map<std::string,Token>::iterator kw = keywords.find(text);
    if( kw == keywords.end() ) return xIdent;
    return kw->second;
  }

  // character sequences
  Token result{xNull};
  switch( c ) {
    case '(': result = xLPar; break;
    case ')': result = xRPar; break;
    case ',': result = xComma; break;
    case '+': result = xAdd; break;
    case '-': result = xSub; break;
    case '*': result = xMul; break;
    case '/': result = xDiv; break;
    case '\\': result = xMod; break;
    case '^': result = xPow; break;
    case '=': result = xEq; break;
    case '<':
      source >> c;
      if( c == '>' )
        result = xNe;
      else if ( c == '=' ) 
        result = xLe;
      else {
        source.unget();
        result = xLt;
      }
      break;
    case '>':
      source >> c;
      if( c == '=' ) 
        result = xGe;
      else {
        source.unget();
        result = xGt;
      }
      break;
    default: 
      break;
  }
  source >> c;

  return result;
}

/**/
std::string Scanner::sequence( std::function<bool(char)> predicate )
{
  std::stringstream buffer;
  while( predicate(c) ) {
    buffer << c;
    source >> c;
  }
  return buffer.str();
}



