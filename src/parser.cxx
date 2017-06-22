
#include "parser.hxx"

namespace basic {
  ///
  Parser::Parser( Scanner& scan )
    : scanner{ scan }
  {}
  
  ///
  Program* Parser::parseProgram()
  {
    scanner >> lookahead;
    
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
    if( lookahead.is(Token::LeftPar) ) {
      match(Token::LeftPar);
      if( lookahead.is(Token::Identifier) ) {
	match(Token::Identifier);
	while( lookahead.is(Token::Comma) ) {
	  match(Token::Comma);
	  match(Token::Identifier);
	}
      }
      match(Token::RightPar);
    }
    parseStatements();
    match(Token::End);
    match(Token::Subroutine);
    return nullptr;
  }

  ///
  Statement* Parser::parseStatements()
  {
    parseNewLines();
    while( true ) {
      if( lookahead.is(Token::Let) )
	parseLet();
      else if( lookahead.is(Token::Input) )
	parseInput();
      else if( lookahead.is(Token::Print) )
	parsePrint();
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
    parseStatements();
    while( lookahead.is(Token::ElseIf) ) {
      match(Token::ElseIf);
      parseExpression();
      match(Token::Then);
      parseStatements();
    }
    if( lookahead.is(Token::Else) ) {
      match(Token::Else);
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
    parseAddition();
    if( lookahead.is({Token::Gt, Token::Ge, Token::Lt, Token::Le, Token::Eq, Token::Ne}) ) {
      scanner >> lookahead;
      parseAddition();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseAddition()
  {
    parseMultiplication();
    while( lookahead.is({Token::Add, Token::Sub, Token::Amp, Token::Or}) ) {
      scanner >> lookahead;
      parseMultiplication();
    }
    return nullptr;
  }

  //
  Expression* Parser::parseMultiplication()
  {
    auto res = parsePower();
    while( lookahead.is({Token::Mul, Token::Div, Token::Mod, Token::And}) ) {
      scanner >> lookahead;
      auto exo = parsePower();
    }
    return nullptr;
  }

  //
  Expression* Parser::parsePower()
  {
    auto res = parseFactor();
    if( lookahead.is(Token::Pow) ) {
      match(Token::Pow);
      auto exo = parseFactor();
      res = new Binary(Operation::Pow, res, exo);
    }
    return res;
  }
  
  ///
  Expression* Parser::parseFactor()
  {
    //
    if( lookahead.is(Token::Number) ) {
      auto lex = lookahead.value;
      match(Token::Number);
      return new Number(std::stod(lex));
    }

    //
    if( lookahead.is(Token::Text) ) {
      auto lex = lookahead.value;
      match(Token::Text);
      return new Text(lex);
    }

    //
    if( lookahead.is({Token::Sub, Token::Not}) ) {
      Operation opc = Operation::None;
      if( lookahead.is(Token::Sub) ) {
	opc = Operation::Sub;
	match(Token::Sub);
      }
      else if( lookahead.is(Token::Not) ) {
	opc = Operation::Not;
	match(Token::Not);
      }
      auto exo = parseFactor();
      return new Unary(opc, exo);
    }
    
    //
    if( lookahead.is(Token::Identifier) ) {
      auto name = lookahead.value;
      match(Token::Identifier);
      if( lookahead.is(Token::LeftPar) ) {
	std::vector<Expression*> args;
	match(Token::LeftPar);
	auto exo = parseExpression();
	args.push_back(exo);
	while( lookahead.is({Token::Number, Token::Text, Token::Identifier,
		Token::Sub, Token::Not, Token::LeftPar}) ) {
	  match(Token::Comma);
	  exo = parseExpression();
	  args.push_back(exo);
	}
	match(Token::RightPar);
	return new Apply(name, args);
      }
      return new Variable(name);
    }

    //
    if( lookahead.is(Token::LeftPar) ) {
      match(Token::LeftPar);
      auto exo = parseExpression();
      match(Token::RightPar);
      return exo;
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

