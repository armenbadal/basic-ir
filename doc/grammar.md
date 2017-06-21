

# BASIC-IR լեզվի քերականությունը (EBNF)

````
Program = { Subroutine NewLines }.
Subroutine = Declaration | Definition.
NewLines = EOL { EOL }.
Declaration = 'DECLARE' SubrHeader.
Definition = SubrHeader StatementList 'END' 'SUB'.
SubrHeader = 'SUB' IDENT '(' [IdentList] ')' NewLines.
StatementList = { Statement NewLines }.
IdentList = IDENT {',' IDENT}.
Statement = Input | Print | Let | If | While | For | Call.
Input = 'INPUT' IDENT.
Print = 'PRINT' Expression.
Let = ['LET'] IDENT '=' Expression.
If = 'IF' Expression 'THEN' NewLines StatementList
     {'ELSEIF' Expression 'THEN' NewLines StatementList }
     ['ELSE' NewLines StatementList] 'END' 'IF'.
While = 'WHILE' Expression NewLines StatementList 'END' 'WHILE'.
For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' Expression]
      NewLines StatementList 'END' 'FOR'.
Call = 'CALL' IDENT '(' [ExpressionList] ')'.
ExpressionList = Expression {',' Expression}.
Expression = Disjunction {'AND' Disjunction}.
Disjunction = Equality {'OR' Equality}.
Equality = Comparison ('=' | '<>') Comparison.
Comparison = Multiplication ('>' | '>=' | '<' | '<=') Multiplication.
Multiplication = Addition {('*' | '/' | '\') Addition}.
Addition = Power {('+' | '-' | '&') Power}.
Power = Factor ['^' Power].
Factor = DOUBLE | STRING | IDENT | '(' Expression ')' 
       | IDENT '(' [ExpressionList] ')'.
````

