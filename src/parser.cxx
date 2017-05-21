
#include "parser.hxx"

namespace basic {
  ///
  Parser::Parser( Scanner& scan )
    :scanner{ scan }
  {}
  
  ///
  Program* Parser::parseProgram()
  {
    while( true ) {
      if( lookahead.is(Token::Declare) )
	parseDeclare();
      else if( lookahead.is(Token::Subroutine) )
	parseSubroutine();
      else
	break;
    }
    return nullptr;
  }

  ///
  Subroutine* Parser::parseDeclare()
  {
    match(Token::Declare);
    parseHeader();
    return nullptr;
  }

  ///
  Subroutine* Parser::parseSubroutine()
  {
    parseHeader();
    //
    match(Token::End);
    match(Token::Subroutine);
    return nullptr;
  }

  ///
  Subroutine* Parser::parseHeader()
  {
    match(Token::Subroutine);
    match(Token::Identifier);
    match(Token::LeftPar);
    if( lookahead.is(Token::Identifier) ) {
      match(Token::Identifier);
      while( lookahead.is(Token::Comma) ) {
	match(Token::Comma);
	match(Token::Identifier);
      }
    }
    match(Token::RightPar);
    parseNewLines();
    return nullptr;
  }

  ///
  Statement* Parser::parseStatement()
  {
    while( true ) {
      if( lookahead.is(Token::Input) )
	parseInput();
      else if( lookahead.is(Token::Print) )
	parsePrint();
      else if( lookahead.is(Token::Let) )
	parseLet();
      else if( lookahead.is(Token::Identifier) )
	parseLet();
      else if( lookahead.is(Token::If) )
	parseIf();
      else if( lookahead.is(Token::While) )
	parseWhile();
      else if( lookahead.is(Token::For) )
	parseFor();
      else if( lookahead.is(Token::Call) )
	parseCall();
      else
	break;
      parseNewLines();
    }
    return nullptr;
  }

  ///
  Input* Parser::parseInput()
  {
    match(Token::Input);
    match(Token::Identifier);
    return nullptr;
  }

  ///
  Print* Parser::parsePrint()
  {
    match(Token::Print);
    // TODO parse expression
    return nullptr;
  }

  ///
  Let* Parser::parseLet()
  {
    if( lookahead.is(Token::Let) )
      scanner >> lookahead;
    match(Token::Identifier);
    match(Token::Eq);
    // TODO parse expression
    return nullptr;
  }
  
  ///
  If* Parser::parseIf()
  {
    match( Token::If );
    // TODO parse expression
    match(Token::Then);
    parseNewLines();
    parseStatement();
    while( lookahead.is(Token::ElseIf) ) {
      match(Token::ElseIf);
      // TODO parse expression
      match(Token::Then);
      parseNewLines();
      parseStatement();
    }
    if( lookahead.is(Token::Else) ) {
      match(Token::Else);
      parseNewLines();
      parseStatement();
    }
    match(Token::End);
    match(Token::If);

    return nullptr;
  }

  ///
  While* Parser::parseWhile()
  {
    match(Token::While);
    // TODO parse expression
    parseNewLines();
    parseStatement();
    match(Token::End);
    match(Token::While);
    return nullptr;
  }

  ///
  For* Parser::parseFor()
  {
    return nullptr;
  }

  ///
  Call* Parser::parseCall()
  {
    return nullptr;
  }

  ///
  void Parser::parseNewLines()
  {
    while (lookahead.is(Token::NewLine))
      scanner >> lookahead;
  }
  
  ///
  void Parser::match(Token tok)
  {
    if (!lookahead.is(tok))
      throw std::string{"Syntax error"};
  }
  
} // basic

