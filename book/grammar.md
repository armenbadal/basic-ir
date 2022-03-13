# BASIC-IR լեզվի քերականությունը (EBNF)

````
Program = [NewLines] { Subroutine NewLines }.
NewLines = EOL { EOL }.
Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] Statements 'END' 'SUB'.
Statements = NewLines { (Let | Input | Print | If | While | For | Call) NewLines }.
IdentList = IDENT {',' IDENT}.
Let = 'LET' IDENT '=' Expression.
Input = 'INPUT' IDENT.
Print = 'PRINT' Expression.
If = 'IF' Expression 'THEN' Statements
     {'ELSEIF' Expression 'THEN' Statements }
     ['ELSE' Statements] 'END' 'IF'.
While = 'WHILE' Expression Statements 'END' 'WHILE'.
For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' ['-'] NUMBER]
      Statements 'END' 'FOR'.
Call = 'CALL' IDENT [ExpressionList].
ExpressionList = Expression {',' Expression}.
Expression = Addition [('=' | '<>' | '>' | '>=' | '<' | '<=') Addition].
Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
Multiplication = Power {('*' | '/' | '\' | 'AND') Power}.
Power = Factor ['^' Power].
Factor = NUMBER | TEXT | IDENT | '(' Expression ')' 
       | IDENT '(' [ExpressionList] ')'.
````

