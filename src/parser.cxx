
#include "parser.hxx"

namespace basic {
  ///
  Parser::Parser( const std::string& filename )
    : scanner{ filename }
  {}
  
  ///
  Program* Parser::parseProgram()
  {
    scanner >> lookahead;
    
    while( lookahead.is(Token::NewLine) )
      match(Token::NewLine);      

    auto prog = new Program("FILENAME");
    while( lookahead.is(Token::Subroutine) ) {
      auto subr = parseSubroutine();
      prog->members.push_back(subr);
      parseNewLines();
    }

    return prog;
  }

  ///
  Subroutine* Parser::parseSubroutine()
  {
    match(Token::Subroutine);
    auto name = lookahead.value;
    match(Token::Identifier);
    std::vector<std::string> params;
    if( lookahead.is(Token::LeftPar) ) {
      match(Token::LeftPar);
      if( lookahead.is(Token::Identifier) ) {
	auto idlex = lookahead.value;
	match(Token::Identifier);
 	params.push_back(idlex);
	while( lookahead.is(Token::Comma) ) {
	  match(Token::Comma);
	  idlex = lookahead.value;
	  match(Token::Identifier);
	  params.push_back(idlex);
	}
      }
      match(Token::RightPar);
    }
    
    auto body = parseStatements();
    match(Token::End);
    match(Token::Subroutine);
    
    return new Subroutine(name, params, body);
  }

  ///
  Statement* Parser::parseStatements()
  {
    parseNewLines();

    auto sequ = new Sequence();
    while( true ) {
      Statement* stat = nullptr;
      if( lookahead.is(Token::Let) )
	stat = parseLet();
      else if( lookahead.is(Token::Input) )
	stat = parseInput();
      else if( lookahead.is(Token::Print) )
	stat = parsePrint();
      else if( lookahead.is(Token::If) )
	stat = parseIf();
      else if( lookahead.is(Token::While) )
	stat = parseWhile();
      else if( lookahead.is(Token::For) )
	stat = parseFor();
      else if( lookahead.is(Token::Call) )
	stat = parseCall();
      else
	break;
      sequ->items.push_back(stat);
      parseNewLines();
    }
    
    return sequ;
  }

  ///
  Statement* Parser::parseLet()
  {
    match(Token::Let);
    auto vnm = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto exo = parseExpression();
    return new Let(vnm, exo);
  }
  
  ///
  Statement* Parser::parseInput()
  {
    match(Token::Input);
    auto vnm = lookahead.value;
    match(Token::Identifier);
    return new Input(vnm);
  }

  ///
  Statement* Parser::parsePrint()
  {
    match(Token::Print);
    auto exo = parseExpression();
    return new Print(exo);
  }

  ///
  Statement* Parser::parseIf()
  {
    match( Token::If );
    auto cond = parseExpression();
    match(Token::Then);
    auto deci = parseStatements();
    auto sif = new If(cond, deci);
    
    auto it = sif;
    while( lookahead.is(Token::ElseIf) ) {
      match(Token::ElseIf);
      auto cone = parseExpression();
      match(Token::Then);
      auto dece = parseStatements();
      auto eif = new If(cone, dece);
      it->alternative = eif;
      it = eif;
    }

    if( lookahead.is(Token::Else) ) {
      match(Token::Else);
      auto alte = parseStatements();
      it->alternative = alte;
    }
    match(Token::End);
    match(Token::If);

    return sif;
  }

  ///
  Statement* Parser::parseWhile()
  {
    match(Token::While);
    auto cond = parseExpression();
    auto body = parseStatements();
    match(Token::End);
    match(Token::While);
    return new While(cond, body);
  }

  ///
  Statement* Parser::parseFor()
  {
    match(Token::For);
    auto par = lookahead.value;
    match(Token::Identifier);
    match(Token::Eq);
    auto be = parseExpression();
    match(Token::To);
    auto en = parseExpression();
    Expression* sp = nullptr;
    if( lookahead.is(Token::Step) ) {
      match(Token::Step);
      bool neg = false;
      if( lookahead.is(Token::Sub) ) {
	match(Token::Sub);
	neg = true;
      }
      auto lex = lookahead.value;
      match(Token::Number);
      sp = new Number(std::stod(lex));
    }
    auto dy = parseStatements();
    match(Token::End);
    match(Token::For);
    return new For(par, be, en, sp, dy);
  }

  ///
  Statement* Parser::parseCall()
  {
    match(Token::Call);
    auto nm = lookahead.value;
    match(Token::Identifier);
    std::vector<Expression*> args;
    if( lookahead.is({Token::Number, Token::Text, Token::Identifier,
	    Token::Sub, Token::Not, Token::LeftPar}) ) {
      auto exo = parseExpression();
      args.push_back(exo);
      while( lookahead.is({Token::Number, Token::Text, Token::Identifier,
	      Token::Sub, Token::Not, Token::LeftPar}) ) {
	match(Token::Comma);
	exo = parseExpression();
	args.push_back(exo);
      }
    }    
    return new Call(nm, args);
  }

  //
  std::map<Token,Operation> mapopcode{
    {Token::Add, Operation::Add},
    {Token::Sub, Operation::Sub},
    {Token::Amp, Operation::Conc},
    {Token::Mul, Operation::Mul},
    {Token::Div, Operation::Div},
    {Token::Mod, Operation::Mod},
    {Token::Pow, Operation::Pow},
    {Token::Eq, Operation::Eq},
    {Token::Ne, Operation::Ne},
    {Token::Gt, Operation::Gt},
    {Token::Ge, Operation::Ge},
    {Token::Lt, Operation::Lt},
    {Token::Le, Operation::Le},
    {Token::And, Operation::And},
    {Token::Or, Operation::Or}
  };
  
  //
  Expression* Parser::parseExpression()
  {
    auto res = parseAddition();
    if( lookahead.is({Token::Gt, Token::Ge, Token::Lt, Token::Le, Token::Eq, Token::Ne}) ) {
      auto opc = mapopcode[lookahead.kind];
      match(lookahead.kind);
      auto exo = parseAddition();
      res = new Binary(opc, res, exo);
    }
    return res;
  }

  //
  Expression* Parser::parseAddition()
  {
    auto res = parseMultiplication();
    while( lookahead.is({Token::Add, Token::Sub, Token::Amp, Token::Or}) ) {
      auto opc = mapopcode[lookahead.kind];
      match(lookahead.kind);
      auto exo = parseMultiplication();
      res = new Binary(opc, res, exo);
    }
    return res;
  }

  //
  Expression* Parser::parseMultiplication()
  {
    auto res = parsePower();
    while( lookahead.is({Token::Mul, Token::Div, Token::Mod, Token::And}) ) {
      auto opc = mapopcode[lookahead.kind];
      match(lookahead.kind);
      auto exo = parsePower();
      res = new Binary(opc, res, exo);
    }
    return res;
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

