# BASIC-IR լեզվի քերականությունը (EBNF)

````
Program = { Subroutine NewLines }.
NewLines = EOL { EOL }.
Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] StatementList 'END' 'SUB'.
StatementList = NewLines { Statement NewLines }.
IdentList = IDENT {',' IDENT}.
Statement = Let | Input | Print | If | While | For | Call.
Input = 'INPUT' IDENT.
Print = 'PRINT' Expression.
Let = 'LET' IDENT '=' Expression.
If = 'IF' Expression 'THEN' StatementList
     {'ELSEIF' Expression 'THEN' StatementList }
     ['ELSE' StatementList] 'END' 'IF'.
While = 'WHILE' Expression StatementList 'END' 'WHILE'.
For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' Expression]
      StatementList 'END' 'FOR'.
Call = 'CALL' IDENT [ExpressionList].
ExpressionList = Expression {',' Expression}.
Expression = Addition [('=' | '<>' | '>' | '>=' | '<' | '<=') Addition].
Addition = Multiplication {('+' | '-' | '&' | 'OR') Multiplication}.
Multiplication = Power {('*' | '/' | '\' | 'AND') Power}.
Power = Factor ['^' Power].
Factor = DOUBLE | STRING | IDENT | '(' Expression ')' 
       | IDENT '(' [ExpressionList] ')'.
````

