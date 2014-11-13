
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
  {"Let", xLet},
  {"If", xIf},
  {"Then", xThen},
  {"ElseIf", xElseIf},
  {"Else", xElse},
  {"For", xFor},
  {"To", xTo},
  {"Step", xStep},
  {"While", xWhile}
};

/**/
Scanner::Scanner( const char* name )
  : source{name}, linenum{-1}, text{""}
{
  source.unsetf( std::ios::skipws );
  c = source.get();
}

/**/
std::string Scanner::lexeme() const
{ return text; }

/**/
Token Scanner::next()
{
  // whitespaces
  while( c == ' ' || c == '\t' ) source >> c;

  // end of file
  if( c == -1 || source.eof() ) return xEof;

  // comments
  if( c == '\'' ) {
    do source >> c; while( c != '\n' );
    ++linenum;
    return xEol;
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
    return xReal;
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



