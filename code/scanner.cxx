
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

  // տեքստի ավարտ
  if( source[position] == '\0' )
    return Lexeme{xEof, "", linenum};
  
  // մեկնաբանություններ
  if( source[position] == '\'' ) {
    while( source[position] != '\n' )
      ++position;
    return next();
  }

  // նոր տողի նիշ
  if( source[position] == '\n' ) {
    ++linenum;
    ++position;
    return Lexeme{xEol, "", linenum};
  }

  // ամբողջ և իրական թվեր
  if( isdigit(source[position]) ) {
    text = sequence( isdigit );
    if( source[position] != '.' )
      return Lexeme{xInteger, text, linenum};
    ++position;
    text += "." + sequence( isdigit );
    return Lexeme{xDouble, text, linenum};
  }

  // իդենտիֆիկատորներ և ծառայողական բառեր
  if( isalpha(source[position]) ) {
    text = sequence( isalnum );
    std::transform(text.begin(), text.end(), text.begin(), tolower);
    if( text == "main" ) // բացառություն մուտքի կետի համար
      text = "Main";
    auto kw = keywords.find(text);
    if( kw == keywords.end() ) 
      return Lexeme{xIdent, text, linenum};
    return Lexeme{kw->second, kw->first, linenum};
  }

  // հործողություններ և մետասիմվոլեր
  Token result = xNull;
  switch( source[position] ) {
    case '(':
      result = xLPar;
      break;
    case ')':
      result = xRPar;
      break;
    case ',':
      result = xComma;
      break;
    case '+':
      result = xAdd;
      break;
    case '-':
      result = xSub;
      break;
    case '*':
      result = xMul;
      break;
    case '/':
      result = xDiv;
      break;
    case '\\':
      result = xMod;
      break;
    case '^':
      result = xPow;
      break;
    case '=':
      result = xEq;
      break;
    case '<':
      ++position;
      if( source[position] == '>' )
        result = xNe;
      else if ( source[position] == '=' ) 
        result = xLe;
      else {
        --position;
        result = xLt;
      }
      break;
    case '>':
      ++position;
      if( source[position] == '=' ) 
        result = xGe;
      else {
	--position;
        result = xGt;
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

