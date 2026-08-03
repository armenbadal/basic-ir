# BASIC-IR Grammar (Niklaus Wirth EBNF)

```ebnf
Program = [NewLines] { Subroutine } .

Subroutine = 'SUB' IDENT [ '(' ParameterList ')' ] ['AS' TypeName] Sequence 'END' 'SUB'.
ParameterList = Parameter { ',' Parameter }.
Parameter = IDENT [ '[' [ Expression ] ']' ] 'AS' TypeName.
TypeName = REAL | TEXT | BOOL.

Sequence   = NewLines { Statement NewLines }.
Statement  = Let | Dim | If | While | For | Call.

Let    = 'LET' IDENT '=' Expression.

Dim    = 'DIM' IDENT [ '[' Expression ']' ] 'AS' TypeName.

If     = 'IF' Expression 'THEN' Sequence
         { 'ELSEIF' Expression 'THEN' Sequence }
         [ 'ELSE' Sequence ]
         'END' 'IF'.

While  = 'WHILE' Expression Sequence 'END' 'WHILE'.

For    = 'FOR' IDENT '=' Expression 'TO' Expression
         [ 'STEP' [ '-' ] Number ]
         Sequence 'END' 'FOR'.

Call   = 'CALL' IDENT [ ExpressionList ].

ExpressionList = Expression { ',' Expression }.

Expression = Addition [ ('=' | '<>' | '>' | '>=' | '<' | '<=') Addition ].
Addition   = Multiplication { ('+' | '-' | '&' | 'OR') Multiplication }.
Multiplication = Power { ('*' | '/' | 'MOD' | 'AND' | '\') Power }.
Power      = Unary [ '^' Power ].
Unary      = { ('+' | '-' | 'NOT') } Subscript.
Subscript  = Factor { '[' Expression ']' }.
Base       = 'TRUE' | 'FALSE' | NUMBER | TEXT | IDENT | Grouped | Apply.
Grouped    = '(' Expression ')'.
Apply      = IDENT '(' [ ExpressionList ] ')'.
```

**Wirth EBNF conventions:**

| Symbol | Meaning |
|--------|---------|
| `=` | definition |
| `\|` | alternation |
| `[ ... ]` | optional |
| `{ ... }` | repetition (zero or more) |
| `'...'` | terminal string |
| `.` | terminator (end of rule) |
| juxtaposition | concatenation |
