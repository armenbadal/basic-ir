
#include "scanner.hxx"

namespace basic {
  
  ///
  std::map<const std::string, Token> Scanner::keywords{
    { "SUB", Token::Subroutine },
    { "LET", Token::Let },
    { "PRINT", Token::Print },
    { "INPUT", Token::Input },
    { "IF", Token::If },
    { "THEN", Token::Then },
    { "ELSEIF", Token::ElseIf },
    { "ELSE", Token::Else },
    { "WHILE", Token::While },
    { "FOR", Token::For },
    { "TO", Token::To },
    { "STEP", Token::Step },
    { "CALL", Token::Call },
    { "AND", Token::And },
    { "OR", Token::Or },
    { "NOT", Token::Not },
  };

  ///
  Scanner::Scanner( const std::string& filename )
  {
    source.open(filename);
    source.unsetf(std::ios_base::skipws);
    source >> ch;
  }
  
  ///
  Scanner::~Scanner()
  {
    if( source.is_open() )
      source.close();
  }
  
  ///
  Scanner& Scanner::operator>>( Lexeme& lex )
  {
    next(lex);
    return *this;
  }
  
  ///
  bool Scanner::next( Lexeme& lex )
  {
    lex.kind = Token::None;
    lex.value = "";
    lex.line = line;

    // skip spaces
    while( ch == ' ' || ch == '\t' )
      source >> ch;

    // detect EOF
    if( source.eof() ) {
      lex.kind = Token::Eof;
      lex.value = "EOF";
      return true;
    }

    //
    if (isdigit(ch))
      return scanNumber(lex);
    
    //
    if (ch == '"')
      return scanText(lex);
    
    //
    if (isalpha(ch))
      return scanIdentifier(lex);

    //
    if( ch == '\'' ) {
      while( ch != '\n' )
	source >> ch;
      return next(lex);
    }
    
    //
    if( ch == '\n' ) {
      lex.kind = Token::NewLine;
      lex.value = "\n";
      ++line;
      source >> ch;
      return true;
    }

    //
    if( ch == '<' ) {
      source >> ch;
      if (ch == '>') {
	source >> ch;
	lex.kind = Token::Ne;
      }
      else if (ch == '=') {
	source >> ch;
	lex.kind = Token::Le;
      }
      else
	lex.kind = Token::Lt;
      // TODO add lex.value
      return true;
    }

    //
    if (ch == '>') {
      source >> ch;
      if (ch == '=') {
	source >> ch;
	lex.kind = Token::Ge;
      }
      else
	lex.kind = Token::Gt;
      // TODO add lex.value
      return true;
    }

    //
    switch( ch ) {
      case '(':
	lex.kind = Token::LeftPar;
	break;
      case ')':
	lex.kind = Token::RightPar;
	break;
      case ',':
	lex.kind = Token::Comma;
	break;
      case '+':
	lex.kind = Token::Add;
	break;
      case '-':
	lex.kind = Token::Sub;
	break;
      case '*':
	lex.kind = Token::Mul;
	break;
      case '/':
	lex.kind = Token::Div;
	break;
      case '\\':
	lex.kind = Token::Mod;
	break;
      case '^':
	lex.kind = Token::Pow;
	break;
      case '=':
	lex.kind = Token::Eq;
	break;
    };

    return lex.kind != Token::None;
  }

  //
  bool Scanner::scanNumber( Lexeme& lex )
  {
    while (isdigit(ch)) {
      lex.value.push_back(ch);
      source >> ch;
    }
    if (ch == '.') {
      source >> ch;
      lex.value.push_back('.');
    }
    while (isdigit(ch)) {
      lex.value.push_back(ch);
      source >> ch;
    }
    lex.kind = Token::Number;
    return true;
  }

  //
  bool Scanner::scanText( Lexeme& lex )
  {
    source >> ch;
    while (ch != '"') {
      lex.value.push_back(ch);
      source >> ch;
    }
    source >> ch;
    lex.kind = Token::Text;
    return true;
  }
  
  //
  bool Scanner::scanIdentifier( Lexeme& lex )
  {
    while (isalnum(ch)) {
      lex.value.push_back(ch);
      source >> ch;
    }
    if (ch == '$') {
      source >> ch;
      lex.value.push_back('$');
    }
    auto ival = keywords.find(lex.value);
    lex.kind = ival == keywords.end() ? Token::Identifier : ival->second;
    return true;
  }
  
  //
  void Scanner::sequence(std::function<bool(char)> pred, std::string& res)
  {
    while( pred(ch) ) {
      res.push_back(ch);
      source >> ch;
    }
  }
} // basic

