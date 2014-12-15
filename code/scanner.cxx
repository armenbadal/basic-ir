
#include <algorithm>
#include <cctype>
#include <sstream>

#include "scanner.hxx"

#include <iostream>


std::map<std::string,Token> Scanner::keywords{
  {"declare", xDeclare},
  {"dim",xDim},
  {"as", xAs},
//{"integer", xIdent},
//{"double", xIdent},
//{"boolean", xIdent},
  {"type", xType},
  {"true", xTrue},
  {"false", xFalse},
  {"end", xEnd},
  {"sub", xSubroutine},
  {"function", xFunction},
  {"return", xReturn},
  {"if", xIf},
  {"then", xThen},
  {"elseif", xElseIf},
  {"else", xElse},
  {"for", xFor},
  {"to", xTo},
  {"step", xStep},
  {"while", xWhile},
  {"input", xInput},
  {"print", xPrint},
  {"and", xAnd},
  {"or", xOr},
  {"not", xNot}
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
  // լեքսեմ
  text = "";

  // բացատանիշեր
  while( c == ' ' || c == '\t' ) source >> c;

  // ֆայլի ավարտ
  if( c == -1 || source.eof() ) return xEof;

  // մեկնաբանություններ
  if( c == '\'' ) {
    while( c != '\n' ) source >> c;
    return next();
  }

  // նոր տողի նիշ
  if( c == '\n' ) {
    ++linenum;
    source >> c;
    return xEol;
  }

  // ամբողջ և իրական թվեր
  if( isdigit(c) ) {
    text = sequence( isdigit );
    if( c != '.' ) return xInteger;
    text += "."; source >> c;
    text += sequence( isdigit );
    return xDouble;
  }

  // իդենտիֆիկատորներ և շառայողական բառեր
  if( isalpha( c ) ) {
    text = sequence( isalnum );
    std::transform(text.begin(), text.end(), text.begin(), tolower);
    if( text == "main" ) text = "Main"; // բացառություն մուտքի կետի համար
    auto kw = keywords.find(text);
    if( kw == keywords.end() ) return xIdent;
    return kw->second;
  }

  // հործողություններ և մետասիմվոլեր
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



