
## BASIC-IR լեզվի շարահյուսությունը

````
Program = { Declare | Function | Subroutine }.
Declare = xDeclare (FuncHeader | SubrHeader).
FuncHeader = xFunction xIdent '(' [ NameDecl { ',' NameDecl } ] ')' xAs xIdent Eols.
SubrHEader = xSubroutine xIDent [ NameDecl { ',' NameDecl } ] Eols.
Function = FuncHeader { Statement } xEnd xFunction Eols.
Soubroutine = SubrHeader { Statements } xEnd xSubroutine Eols.
Statement = xPrint Disjunction { ',' Disjunction } Eols
    | xInput xIdent { ',' xIdent } Eols
    | xWhile Disjunction Eols 
      { Statement } xEnd xWhile Eols
    | xFor xIdent '=' Addition xTo Addition [ xStep Addition ] Eols 
      { Statement } xEnd xFor  Eols
    | xIf Disjunction xThen Eols { Statement } 
      { xElseIf Disjunction xThen Eols { Statement } } 
      [xElse Eols { Statement }] xEnd xIf Eols
    | xIdent '=' Disjunction Eols
    | xIdent [Disjunction { ',' Disjunction }] Eols
    | xDim NameDecl Eols
    | xReturn Disjunction Eols.
NameDecl = xIdent xAs xIdent.
Disjunction = Conjunction { xOr Conjunction }.
Conjunction = Equality { xAnd Equality }.
Equality = Relation [ ('='|'<>') Relation ].
Relation = Addition [ ('>'|'>='|'<'|'<=') Addition ].
Addition = Multiplication { ('+'|'-') Multiplication }
Multiplication = Power { ('*'|'/'|'\\') Power }.
Power = Factor ['^' Power].
Factor = xIdent
    | xIdent '(' [NameDecl { ',' NameDecl }] ')'
    | xInteger
    | xDouble
    | xTrue
    | xFalse
    | xSub Factor
    | xNot Factor
    | '(' Relation ')'.
````


