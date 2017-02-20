
#include <algorithm>
#include <cctype>
#include <sstream>

#include "scanner.hxx"

#include <iostream>


std::map<std::string,Token> Scanner::keywords{
  {"declare", Token::xDeclare},
  {"dim", Token::xDim},
  {"as", Token::xAs},
//{"integer", xIdent},
//{"double", xIdent},
//{"boolean", xIdent},
  {"type", Token::xType},
  {"true", Token::xTrue},
  {"false", Token::xFalse},
  {"end", Token::xEnd},
  {"sub", Token::xSubroutine},
  {"function", Token::xFunction},
  {"return", Token::xReturn},
  {"if", Token::xIf},
  {"then", Token::xThen},
  {"elseif", Token::xElseIf},
  {"else", Token::xElse},
  {"for", Token::xFor},
  {"to", Token::xTo},
  {"step", Token::xStep},
  {"while", Token::xWhile},
  {"input", Token::xInput},
  {"print", Token::xPrint},
  {"and", Token::xAnd},
  {"or", Token::xOr},
  {"not", Token::xNot}
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
  if( c == -1 || source.eof() ) return Token::xEof;

  // մեկնաբանություններ
  if( c == '\'' ) {
    while( c != '\n' ) source >> c;
    return next();
  }

  // նոր տողի նիշ
  if( c == '\n' ) {
    ++linenum;
    source >> c;
    return Token::xEol;
  }

  // ամբողջ և իրական թվեր
  if( isdigit(c) ) {
    text = sequence( isdigit );
    if( c != '.' ) return Token::xInteger;
    text += "."; source >> c;
    text += sequence( isdigit );
    return Token::xDouble;
  }

  // իդենտիֆիկատորներ և շառայողական բառեր
  if( isalpha( c ) ) {
    text = sequence( isalnum );
    std::transform(text.begin(), text.end(), text.begin(), tolower);
    if( text == "main" ) text = "Main"; // բացառություն մուտքի կետի համար
    auto kw = keywords.find(text);
    if( kw == keywords.end() ) return Token::xIdent;
    return kw->second;
  }

  // գործողություններ և մետասիմվոլեր
  Token result = Token::xNull;
  switch( c ) {
    case '(': result = Token::xLPar; break;
    case ')': result = Token::xRPar; break;
    case ',': result = Token::xComma; break;
    case '+': result = Token::xAdd; break;
    case '-': result = Token::xSub; break;
    case '*': result = Token::xMul; break;
    case '/': result = Token::xDiv; break;
    case '\\': result = Token::xMod; break;
    case '^': result = Token::xPow; break;
    case '=': result = Token::xEq; break;
    case '<':
      source >> c;
      if( c == '>' )
        result = Token::xNe;
      else if ( c == '=' ) 
        result = Token::xLe;
      else {
        source.unget();
        result = Token::xLt;
      }
      break;
    case '>':
      source >> c;
      if( c == '=' ) 
        result = Token::xGe;
      else {
        source.unget();
        result = Token::xGt;
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



