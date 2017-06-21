
#include "parser.hxx"

namespace basic {
  ///
  Parser::Parser( Scanner& scan )
    :scanner{ scan }
  {}
  
  ///
  Program* Parser::parseProgram()
  {
    while( lookahead.is(Token::NewLine) )
      match(Token::NewLine);      
    
    while( lookahead.is(Token::Subroutine) ) {
      parseSubroutine();
      parseNewLines();
    }

    return nullptr;
  }

  ///
  Subroutine* Parser::parseSubroutine()
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

    match(Token::End);
    match(Token::Subroutine);
    return nullptr;
  }

  ///
  Statement* Parser::parseStatements()
  {
    while( true ) {
      if( lookahead.is(Token::Let) )
	parseLet();
      else if( lookahead.is(Token::Input) )
	parseInput();
      else if( lookahead.is(Token::Print) )
	parsePrint();
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
  Statement* Parser::parseLet()
  {
    match(Token::Let);
    match(Token::Identifier);
    match(Token::Eq);
    parseExpression();
    return nullptr;
  }
  
  ///
  Statement* Parser::parseInput()
  {
    match(Token::Input);
    match(Token::Identifier);
    return nullptr;
  }

  ///
  Statement* Parser::parsePrint()
  {
    match(Token::Print);
    parseExpression();
    return nullptr;
  }

  ///
  Statement* Parser::parseIf()
  {
    match( Token::If );
    parseExpression();
    match(Token::Then);
    parseNewLines();
    parseStatements();
    while( lookahead.is(Token::ElseIf) ) {
      match(Token::ElseIf);
      parseExpression();
      match(Token::Then);
      parseNewLines();
      parseStatements();
    }
    if( lookahead.is(Token::Else) ) {
      match(Token::Else);
      parseNewLines();
      parseStatements();
    }
    match(Token::End);
    match(Token::If);

    return nullptr;
  }

  ///
  Statement* Parser::parseWhile()
  {
    match(Token::While);
    parseExpression();
    parseNewLines();
    parseStatements();
    match(Token::End);
    match(Token::While);
    return nullptr;
  }

  ///
  Statement* Parser::parseFor()
  {
    return nullptr;
  }

  ///
  Statement* Parser::parseCall()
  {
    match(Token::Call);
    // TODO
    return nullptr;
  }

  //
  Expression* Parser::parseExpression()
  {
    parseConjunction();
    while( lookahead.is(Token::Or) ) {
      scanner >> lookahead;
      parseConjunction();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseConjunction()
  {
    parseEquality();
    while( lookahead.is(Token::And) ) {
      scanner >> lookahead;
      parseEquality();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseEquality()
  {
    parseComparison();
    if( lookahead.is({Token::Eq, Token::Ne}) ) {
      scanner >> lookahead;
      parseComparison();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseComparison()
  {
    parseAddition();
    if( lookahead.is({Token::Gt, Token::Ge, Token::Lt, Token::Le}) ) {
      scanner >> lookahead;
      parseAddition();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseAddition()
  {
    parseMultiplication();
    while( lookahead.is({Token::Add, Token::Sub, Token::Amp}) ) {
      scanner >> lookahead;
      parseMultiplication();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseMultiplication()
  {
    parsePower();
    while( lookahead.is({Token::Mul, Token::Div, Token::Mod}) ) {
      scanner >> lookahead;
      parsePower();
    }
    return nullptr;
  }

  //
  Expression* Parser::parsePower()
  {
    parseFactor();
    if( lookahead.is(Token::Pow) ) {
      match(Token::Pow);
      parseFactor();
    }
    return nullptr;
  }
  
  //
  // Factor = DOUBLE
  //        | STRING
  //        | IDENT
  //        | IDENT '(' [ExpressionList] ')'
  //        | '(' Expression ')'.
  //
  Expression* Parser::parseFactor()
  {
    //
    if( lookahead.is(Token::Number) ) {
      match(Token::Number);
      return nullptr;
    }

    //
    if( lookahead.is(Token::Text) ) {
      match(Token::Text);
      return nullptr;
    }

    //
    if( lookahead.is({Token::Sub, Token::Not}) ) {
      match(lookahead.kind);
      return nullptr;
    }
    
    //
    if( lookahead.is(Token::Identifier) ) {
      match(Token::Identifier);
      if( lookahead.is(Token::LeftPar) ) {
	match(Token::LeftPar);
	parseExpression();
	while( lookahead.is({Token::Number, Token::Text, Token::Identifier,
		Token::Sub, Token::Not, Token::LeftPar}) ) {
	  match(Token::Comma);
	  parseExpression();
	}
	match(Token::RightPar);
	return nullptr;
      }
      return nullptr;
    }

    //
    if( lookahead.is(Token::LeftPar) ) {
      match(Token::LeftPar);
      parseExpression();
      match(Token::RightPar);
      return nullptr;
    }

    return nullptr;
  }
  
  ///
  void Parser::parseNewLines()
  {
    match(Token::NewLine);
    while( lookahead.is(Token::NewLine) )
      match(Token::NewLine);
  }
  
  ///
  void Parser::match( Token exp )
  {
    if( !lookahead.is(exp) )
      throw std::string{"Syntax error"};

    scanner >> lookahead;
  }  
} // basic

