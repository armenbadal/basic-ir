
#include <algorithm>
#include <cctype>
#include <sstream>

#include "scanner.hxx"

#include <iostream>

namespace basic {
  std::map<std::string,Token> Scanner::keywords{
    {"declare",  xDeclare},
    {"dim",      xDim},
    {"as",       xAs},
  //{"integer",  xIdent},
  //{"double",   xIdent},
  //{"boolean",  xIdent},
    {"type",     xType},
    {"true",     xTrue},
    {"false",    xFalse},
    {"end",      xEnd},
    {"sub",      xSubroutine},
    {"function", xFunction},
    {"return",   xReturn},
    {"if",       xIf},
    {"then",     xThen},
    {"elseif",   xElseIf},
    {"else",     xElse},
    {"for",      xFor},
    {"to",       xTo},
    {"step",     xStep},
    {"while",    xWhile},
    {"input",    xInput},
    {"print",    xPrint},
    {"and",      xAnd},
    {"or",       xOr},
    {"not",      xNot}
  };
}

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
basic::Scanner::Scanner( const std::string& name )
{
  std::ifstream inps{name};
  // TODO ստուգել ֆայլի հաջող բացված լինելը

  // հաշվել ֆայլի երկարությունը
  inps.seekg(0, inps.end);
  int size = inps.tellg();
  inps.seekg(0, inps.beg);
  
  // կարդալ ֆայլի պարունակությունը
  source = new char[size+1];
  inps.read(source, size);
  source[size] = '\0';

  // փակել ֆայլի հոսքը
  inps.close();
}

/**/
basic::Scanner::~Scanner()
{
  delete[] source;
}

/**/
basic::Lexeme basic::Scanner::next()
{
  // լեքսեմ
  text = "";

  // բացատանիշեր
  while( source[position] == ' ' || source[position] == '\t' )
    ++position;

  // ֆայլի ավարտ
  if( c == -1 || source.eof() ) return Token::xEof;

  // մեկնաբանություններ
  if( source[position] == '\'' ) {
    while( source[position] != '\n' )
      ++position;
    return next();
  }

  // նոր տողի նիշ
  if( source[position] == '\n' ) {
    ++linenum;
    source >> c;
    return Token::xEol;
  }

  // ամբողջ և իրական թվեր
  if( isdigit(source[position]) ) {
    text = sequence( isdigit );
    if( c != '.' ) return Token::xInteger;
    text += "."; source >> c;
    text += sequence( isdigit );
    return Token::xDouble;
  }

  // իդենտիֆիկատորներ և ծառայողական բառեր
  if( isalpha(source[position]) ) {
    text = sequence( isalnum );
    std::transform(text.begin(), text.end(), text.begin(), tolower);
    if( text == "main" ) // բացառություն մուտքի կետի համար
      text = "Main";
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
  ++position;

  return Lexeme{result, "", linenum};
}

/**/
std::string basic::Scanner::sequence( std::function<bool(char)> predicate )
{
  std::string res = "";
  while( predicate(source[position]) ) {
    res.push_back(source[position]);
    ++position;
  }
  return res;
}

