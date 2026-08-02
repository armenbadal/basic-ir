# BASIC-IR Grammar (Niklaus Wirth EBNF)

```ebnf
Program = { Subroutine } .

Subroutine = 'SUB' IDENT [ '(' IdentList ')' ] Sequence 'END' 'SUB' .
IdentList  = IDENT { ',' IDENT } .

Sequence   = { Statement } .
Statement  = Let | Dim | If | While | For | Call .

Let    = 'LET' IDENT '=' Expression .
Dim    = 'DIM' IDENT '[' Expression ']' 'AS' (REAL | TEXT | BOOL) .

If     = 'IF' Expression 'THEN' Sequence
         { 'ELSEIF' Expression 'THEN' Sequence }
         [ 'ELSE' Sequence ]
         'END' 'IF' .

While  = 'WHILE' Expression Sequence 'END' 'WHILE' .

For    = 'FOR' IDENT '=' Expression 'TO' Expression
         [ 'STEP' [ '-' ] Number ]
         Sequence 'END' 'FOR' .

Call   = 'CALL' IDENT [ ExpressionList ] .
ExpressionList = Expression { ',' Expression } .

Expression = Addition [ CompOp Addition ] .
Addition   = Multiplication { AddOp Multiplication } .
Multiplication = Power { MulOp Power } .
Power      = Factor [ '^' Power ] .
Unary      = { UnaryOp } Base .
Base       = TrueOrFalse | Number | Text | IDENT | Grouped | Apply .
Grouped    = '(' Expression ')' .
Apply      = IDENT '(' [ ExpressionList ] ')' .

TrueOrFalse = 'TRUE' | 'FALSE' .
Number      = NUMBER .
Text        = TEXT .

CompOp = '=' | '<>' | '>' | '>=' | '<' | '<=' .
AddOp  = '+' | '-' | '&' | 'OR' .
MulOp  = '*' | '/' | 'MOD' | 'AND' | QUOT .
UnaryOp = '+' | '-' | 'NOT' .
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
