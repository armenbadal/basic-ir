# Շարահյուսական վերլուծությունը

Այս փաստաթուղթը մանրամասն նկարագրում է `basic-ir` կոմպիլյատորի շարահյուսական
վերլուծիչի (parser) իրականացումը և վերլուծության ընթացքում սխալների
հայտնաբերման, մշակման ու վերլուծության վերսկսման եղանակը։

Վերաբերող ֆայլերն են՝

| Ֆայլ | Բովանդակությունը |
|------|------------------|
| [src/parser.hxx](src/parser.hxx) | `Parser` դասի հայտարարությունը |
| [src/parser.cxx](src/parser.cxx) | վերլուծիչի իրականացումը |
| [src/diagnostics.hxx](src/diagnostics.hxx) | `SyntaxError` կառուցվածքը և `Diagnostics` հավաքիչը |
| [src/diagnostics.cxx](src/diagnostics.cxx) | սխալների գրանցման կանոնները |
| [src/scanner.hxx](src/scanner.hxx) | բառային վերլուծիչը |
| [src/lexeme.hxx](src/lexeme.hxx) | `Token` պիտակները և `Lexeme` դասը |
| [src/ast.hxx](src/ast.hxx) | շարահյուսական ծառի հանգույցները |
| [book/grammar.md](book/grammar.md) | լեզվի քերականությունը |

---

## Բովանդակություն

1. [Վերլուծիչի իրականացումը](#1-վերլուծիչի-իրականացումը)
2. [Սխալների հայտնաբերումը, մշակումը և վերլուծության վերսկսումը](#2-սխալների-հայտնաբերումը-մշակումը-և-վերլուծության-վերսկսումը)
3. [Գրականություն և հղումներ](#3-գրականություն-և-հղումներ)

---

## 1. Վերլուծիչի իրականացումը

### 1.1. Ընդհանուր սխեման

Կոմպիլյատորի առաջին երկու փուլերը կազմում են պարզ շղթա.

```
ֆայլ → Scanner → Lexeme-ների հոսք → Parser → շարահյուսական ծառ (AST)
```

`Scanner`-ը (տե՛ս [src/scanner.hxx](src/scanner.hxx)) նիշերի հոսքից կարդում է
_լեքսեմներ_։ Յուրաքանչյուր լեքսեմ ([src/lexeme.hxx](src/lexeme.hxx)) երեք
բաղադրիչ ունի.

```cpp
class Lexeme {
public:
    Token kind = Token::None; // պիտակը
    std::string value;        // տեքստը (լեքսեմը)
    unsigned int line = 0;    // տողի համարը
};
```

`Parser`-ը լեքսեմների այս հոսքից կառուցում է _աբստրակտ շարահյուսական ծառ_
(abstract syntax tree, AST), որի հանգույցները սահմանված են
[src/ast.hxx](src/ast.hxx)-ում։

Կարևոր է, որ վերլուծիչը լեքսեմների ամբողջ հոսքը նախապես չի կուտակում։ Ցանկացած
պահի հիշվում է **ընդամենը մեկ լեքսեմ** — հերթական չմշակվածը.

```cpp
Scanner& scanner;
Lexeme lookahead;
```

### 1.2. Ռեկուրսիվ իջնող վերլուծություն

Օգտագործվում է վերլուծության ամենապարզ և ամենաընթեռնելի եղանակը՝ **ռեկուրսիվ
իջնող վերլուծությունը** (recursive descent parsing)։ Դրա գաղափարը մեկ
նախադասությամբ ձևակերպվում է այսպես.

> Քերականության յուրաքանչյուր ոչ տերմինալ սիմվոլին համապատասխանեցվում է մեկ
> ֆունկցիա, որի մարմինը կրկնում է այդ սիմվոլի աջ մասի կառուցվածքը։

Այսինքն՝ քերականությունը ուղղակիորեն «թարգմանվում» է կոդի։ Հենց սա է
[book/grammar.md](book/grammar.md)-ի և [src/parser.cxx](src/parser.cxx)-ի միջև
եղած մեկ-մեկ համապատասխանության պատճառը։ Ամեն վերլուծող ֆունկցիայի վերևում
գրված է քերականական այն կանոնը, որը նա իրականացնում է.

```cpp
// Subroutine = 'SUB' IDENT ['(' [IdentList] ')'] Statements 'END' 'SUB'.
Subroutine::Ptr Parser::parseSubroutine()
```

Այս եղանակի առավելությունը դասընթացի տեսանկյունից այն է, որ վերլուծիչը
_կարդացվում է_. կոդն ինքն է քերականության փաստաթուղթը։ Թերությունը՝ ձախ
ռեկուրսիա պարունակող քերականությունների հետ ուղղակիորեն չի աշխատում (այս մասին՝
1.7 բաժնում)։

### 1.3. Քերականությունից դեպի կոդ. թարգմանության կանոնները

Ռեկուրսիվ իջնող վերլուծիչ գրելը մեխանիկական աշխատանք է։ EBNF-ի յուրաքանչյուր
կառուցվածքին համապատասխանում է C++-ի որոշակի կառուցվածք.

| EBNF | C++ |
|------|-----|
| `a` (տերմինալ) | `match(Token::A)` |
| `A` (ոչ տերմինալ) | `parseA()` |
| `a b` (հաջորդականություն) | `match(Token::A); parseB();` |
| `a \| b` (ընտրություն) | `if( lookahead.is(Token::A) ) ... else ...` |
| `[a]` (ոչ պարտադիր) | `if( lookahead.is(Token::A) ) ...` |
| `{a}` (կրկնություն) | `while( lookahead.is(Token::A) ) ...` |

Օրինակ՝ `Dim = 'DIM' IDENT '[' Expression ']'.` կանոնը դառնում է.

```cpp
Dim::Ptr Parser::parseDim()
{
    auto line = lookahead.line;

    match(Token::Dim);
    auto name = match(Token::Identifier);
    match(Token::LeftBrack);
    auto size = parseExpression();
    match(Token::RightBrack);

    return node<Dim>(name, size, line);
}
```

### 1.4. Մեկ թոքեն առաջ նայելը

Վերլուծիչը **LL(1)** դասի է. որոշում կայացնելու համար նրան բավական է միայն
ընթացիկ (առաջ նայված, _lookahead_) թոքենը։ Դա նշանակում է, որ ամեն ընտրության
կետում այլընտրանքների FIRST բազմությունները չեն հատվում։

Հոսքով առաջ շարժվելու միակ ֆունկցիան է.

```cpp
void Parser::advance()
{
    lookahead = scanner.scan();
    diagnostics.advance();
}
```

Երկրորդ տողն ինքնին վերլուծության համար պետք չէ. այն սխալների հավաքիչին
հայտնում է, որ վերլուծիչն առաջ է շարժվել։ Այս ազդանշանը կենտրոնական դեր է
խաղում սխալների զտման մեջ (տե՛ս 2.3)։

Թոքենի ստուգման ու կլանման համար կա մեկ ֆունկցիա.

```cpp
std::string Parser::match(Token exp)
{
    if( !lookahead.is(exp) ) {
        diagnostics.mark(lookahead.line, std::format("Սպասվում է '{}', բայց հանդիպել է {}։", toString(exp), describe(lookahead)));
        return {};
    }

    const auto value = lookahead.value;
    advance();
    return value;
}
```

Կանչերի մեծ մասին թոքենի _տեքստը_ պետք չէ (բանալի բառեր, կետադրական
նշաններ) — նրանք պարզապես անտեսում են վերադարձվող արժեքը.

```cpp
match(Token::Let);
```

Իսկ երբ տեքստը պետք է (իդենտիֆիկատորներ, հաստատուններ), այն վերցվում է
վերադարձից.

```cpp
auto name = match(Token::Identifier);
```

Ամենակարևորը՝ `match()`-ը սխալի դեպքում **չի ընդհատում վերլուծությունը** —
այս մասին մանրամասն 2-րդ բաժնում։

### 1.5. FIRST բազմությունները

Երբ ընտրությունը մեկ թոքենով չի որոշվում (օրինակ՝ «հրաման կա՞, թե՞
հաջորդականությունն ավարտվեց»), օգտագործվում են բացահայտ բազմություններ.

```cpp
const std::set FirstStat = {
    Token::Let, Token::Dim,
    Token::If, Token::While, Token::For, Token::Call
};
const std::set FirstExpr = {
    Token::True, Token::False, Token::Number, Token::Text,
    Token::Identifier, Token::Sub, Token::Not, Token::LeftPar
};

// Թոքենները, որոնցով ավարտվում է հրամանների հաջորդականությունը
const std::set FollowStat = {
    Token::End, Token::ElseIf, Token::Else,
    Token::Subroutine, Token::Eof
};
```

`FirstStat`-ը հրաման սկսող թոքենների բազմությունն է, `FirstExpr`-ը՝
արտահայտություն սկսողների, իսկ `FollowStat`-ը հրամանների հաջորդականությանը
_հաջորդող_ թոքենների բազմությունն է։ Վերջինս է որոշում, թե որտեղ է ավարտվում
բլոկը։

### 1.6. Հրամանների վերլուծությունը

Ծրագիրը ենթածրագրերի հաջորդականություն է.

```cpp
// Program = [NewLines] { Subroutine NewLines }.
Program::Ptr Parser::parseProgram()
{
    // առաջին թոքենը
    advance();
    auto line = lookahead.line;

    // [NewLines]. ֆայլի սկզբում դատարկ տողերը պարտադիր չեն
    if( lookahead.is(Token::NewLine) )
        parseNewLines();

    std::vector<Subroutine::Ptr> subroutines;
    while( !lookahead.is(Token::Eof) ) {
        // sync
        sync(SubroutineSync, ...);

        if( lookahead.is(Token::Subroutine) )
            subroutines.push_back(parseSubroutine());

        parseNewLines();
    }

    return node<Program>(std::move(subroutines), line);
}
```

Ուշադրություն դարձրեք, որ առաջին `advance()`-ը կանչվում է հենց այստեղ՝
կոնստրուկտորում չէ։ Դա նշանակում է, որ `Parser`-ի ստեղծումն ինքնին ոչինչ չի
կարդում. վերլուծությունն սկսվում է միայն `parse()`-ի կանչից։

Հրամանների հաջորդականությունը վերլուծվում է `parseSequence()`-ով, որն ընթանում
է այնքան, մինչև հանդիպի `FollowStat`-ի թոքեն.

```cpp
// Statements = NewLines { Statement NewLines }.
Sequence::Ptr Parser::parseSequence()
{
    auto line = lookahead.line;
    parseNewLines();

    std::vector<Statement::Ptr> items;
    while( !FollowStat.contains(lookahead.kind) ) {
        if( lookahead.is(Token::NewLine) ) {
            parseNewLines();
            continue;
        }

        // sync
        sync(StatementSync, ...);

        if( FirstStat.contains(lookahead.kind) ) {
            items.push_back(parseOneStatement());
            parseNewLines();
        }
    }

    return node<Sequence>(std::move(items), line);
}
```

`parseOneStatement()`-ը սովորական դիսպետչեր է. ըստ ընթացիկ թոքենի ընտրում է
համապատասխան վերլուծող ֆունկցիան (`parseLet`, `parseIf`, `parseWhile` և այլն)։

Տողի ավարտը մշակվում է `parseNewLines()`-ով.

```cpp
// NewLines = NL { NL }.
void Parser::parseNewLines()
{
    // ֆայլի ավարտն ինքնին տողի ավարտ է
    if( !lookahead.is(Token::NewLine, Token::Eof) )
        diagnostics.mark(lookahead.line, std::format("Սպասվում է տողի ավարտ, բայց հանդիպել է {}։", describe(lookahead)));

    while( lookahead.is(Token::NewLine) )
        advance();
}
```

Քերականությունն ասում է `NewLines = EOL { EOL }.` — **առնվազն մեկ**։ Ուստի
տողի ավարտի բացակայությունը սխալ է. առանց այս ստուգման `LET a = 1 LET b = 2`
տողը լուռ կընդունվեր որպես երկու հրաման։

Այն մեկ տեղում, որտեղ քերականությունը դատարկ տողերը թույլ է տալիս, բայց չի
պարտադրում՝ `Program = [NewLines] { Subroutine NewLines }.`, կանչը
փակագծվում է հենց այն `if`-ով, որին համապատասխանում է `[...]`-ը (տե՛ս 1.3-ի
աղյուսակը).

```cpp
// [NewLines]. ֆայլի սկզբում դատարկ տողերը պարտադիր չեն
if( lookahead.is(Token::NewLine) )
    parseNewLines();
```

### 1.7. Արտահայտությունները և գործողությունների առաջնահերթությունը

Արտահայտությունների քերականությունը սովորաբար ձախ ռեկուրսիվ է.

```
Addition = Addition '+' Multiplication | Multiplication.
```

Ռեկուրսիվ իջնող վերլուծիչում այս կանոնն ուղղակիորեն գրելը կտար անվերջ
ռեկուրսիա (`parseAddition()`-ն առաջին քայլով կկանչեր ինքն իրեն)։ Ուստի ձախ
ռեկուրսիան փոխարինվում է կրկնությամբ.

```
Addition = Multiplication { '+' Multiplication }.
```

Առաջնահերթությունն արտահայտվում է ֆունկցիաների **բնադրման խորությամբ**. որքան
ցածր է ֆունկցիան շղթայում, այնքան բարձր է գործողության առաջնահերթությունը։

| Ֆունկցիա | Գործողություններ | Ասոցիատիվություն |
|----------|------------------|------------------|
| `parseExpression` | `=` `<>` `<` `<=` `>` `>=` | ոչ ասոցիատիվ (միայն մեկ անգամ) |
| `parseAddition` | `+` `-` `&` `OR` | ձախ |
| `parseMultiplication` | `*` `/` `MOD` `\` `AND` | ձախ |
| `parsePower` | `^` | **աջ** |
| `parseUnary` | ունար `+` `-` `NOT` | պրեֆիքս |
| `parseSubscript` | `[...]` | ձախ |
| `parseFactor` | հաստատուններ, փոփոխականներ, `( )` | — |

Ձախ ասոցիատիվությունը ստացվում է `while` ցիկլով.

```cpp
Expression::Ptr Parser::parseAddition()
{
    auto res = parseMultiplication();
    while( lookahead.is(Token::Add, Token::Sub, Token::Amp, Token::Or) ) {
        auto opc = opCode(lookahead.kind);
        auto ln = lookahead.line;
        match(lookahead.kind);
        auto exo = parseMultiplication();
        res = node<Binary>(opc, res, exo, ln);  // res-ը դառնում է ձախ ենթածառ
    }
    return res;
}
```

Աջ ասոցիատիվությունը՝ ռեկուրսիայով.

```cpp
Expression::Ptr Parser::parsePower()
{
    auto res = parseUnary();
    if( lookahead.is(Token::Pow) ) {
        auto ln = lookahead.line;
        match(Token::Pow);
        auto exo = parsePower();  // ինքն իրեն, ոչ թե parseUnary
        res = node<Binary>(Operation::Pow, res, exo, ln);
    }
    return res;
}
```

Այսպիսով՝ `2 ^ 3 ^ 2` վերլուծվում է որպես `2 ^ (3 ^ 2)`, իսկ `1 - 2 - 3`՝
որպես `(1 - 2) - 3`։

### 1.8. Ծառի կառուցումը

Հանգույցները ստեղծվում են [src/ast.hxx](src/ast.hxx)-ի օժանդակ ֆունկցիայով.

```cpp
template<typename P, typename... Args>
std::shared_ptr<P> node(Args&&... args);
```

Ամեն հանգույց հիշում է իր դիրքը սկզբնական տեքստում (`using Position = unsigned int;`՝
տողի համարը)։ Դրա համար էլ գրեթե ամեն վերլուծող ֆունկցիա սկսվում է
`auto line = lookahead.line;` տողով — դիրքը վերցվում է **կառուցվածքի առաջին
թոքենից**, ոչ թե վերջինից։ Այս տեղեկությունն անհրաժեշտ է հետագա փուլերին
(իմաստային ստուգումներ, շտկման ինֆորմացիա)։

---

## 2. Սխալների հայտնաբերումը, մշակումը և վերլուծության վերսկսումը

### 2.1. Խնդրի ձևակերպումը

Ամենապարզ վարքագիծը՝ առաջին սխալի վրա կանգնելը, գործնականում անհարմար է.
օգտագործողը ստիպված է լինում կոմպիլյատորը կանչել այնքան անգամ, քանի սխալ կա
ֆայլում։ Ուստի վերլուծիչից պահանջվում է **մեկ անցումով գտնել հնարավորինս շատ
իրարից անկախ սխալ**։

Բայց այստեղ առաջանում է հակասություն.

- եթե սխալից հետո վերլուծիչը շարունակի այնպես, կարծես ոչինչ չի եղել, նա կմտնի
  «անհամաձայնեցված» վիճակ և կսկսի հաղորդել **կեղծ սխալներ**՝ առաջինի
  հետևանքները (այս երևույթը կոչվում է _սխալների հեղեղ_, error cascade),
- եթե չափից շատ բան բաց թողնի, կկորցնի **իրական սխալները**։

Ուստի սխալների մշակման ցանկացած եղանակ պետք է պատասխանի երեք հարցի.

1. **Ինչպե՞ս հաղորդել** սխալը (և որո՞նք չհաղորդել)։
2. **Ինչպե՞ս շարունակել** վերլուծությունը սխալից հետո։
3. **Ինչպե՞ս երաշխավորել**, որ վերլուծիչը չի կախվի անվերջ ցիկլում։

### 2.2. Վիրտի եղանակը

Այս իրականացումը հետևում է Նիկլաուս Վիրտի «Compiler Construction» գրքի 7.3
բաժնում («Coping with syntactic errors») նկարագրված եղանակին։ Դրա գլխավոր
առանձնահատկությունն այն է, որ **բացառություններ (exceptions) չեն օգտագործվում**։
Սխալի հաղորդումը երբեք չի ընդհատում կառավարման սովորական հոսքը։

Ամբողջ մեխանիզմը երեք փոքր ֆունկցիա է.

| Ֆունկցիա | Վիրտի մոտ | Դերը |
|----------|-----------|------|
| `Diagnostics::mark()` | `Mark` | գրանցում է սխալը՝ զտելով կրկնությունները |
| `match()` | `CheckSymbol` | ստուգում է թոքենը, սխալի դեպքում շարունակում է այնպես, կարծես այն կար |
| `sync()` | համաժամեցման կետեր | բաց է թողնում ավելորդ թոքենները մինչև «անվտանգ» կետը |

### 2.3. `Diagnostics::mark()` — սխալի գրանցումը

Վիրտի բնագրում (Oberon կոմպիլյատորի `ORS.Mod` մոդուլից) `Mark`-ը գրված է
այսպես.

```oberon
PROCEDURE Mark*(msg: ARRAY OF CHAR);
  VAR p: LONGINT;
BEGIN p := Pos();
  IF (p > errpos) & (errcnt < 25) THEN
    ... տպել սխալը ...
  END ;
  INC(errcnt); errpos := p + 4
END Mark;
```

Ամբողջ իմաստը `p > errpos` պայմանի մեջ է. **սխալը հաղորդվում է միայն այն
դեպքում, երբ վերլուծիչն առաջ է շարժվել վերջին հաղորդված սխալից հետո**։ Եթե
վերլուծիչը տեղից չի շարժվել, ուրեմն նոր հաղորդագրությունը նույն սխալի մասին է։

Այս մեկ պայմանը փոխարինում է «խուճապի ռեժիմի» (panic mode) դրոշին, որը
սովորաբար օգտագործվում է նույն նպատակով։

Վիրտին դիրքերը համեմատել է պետք, որովհետև իր `Mark`-ը դիրքը վերցնում է
բառային վերլուծիչից և «առաջ շարժվելու» մասին առանձին ազդանշան չունի։ Մեր
վերլուծիչում, սակայն, թոքեն կլանելու **միակ** կետը `advance()`-ն է, ուստի
համեմատության փոխարեն բավական է մեկ դրոշ ([src/diagnostics.cxx](src/diagnostics.cxx)).

```cpp
void Diagnostics::mark(unsigned int line, std::string_view message)
{
    if( !_advanced )
        return;

    _advanced = false;
    ++_count;

    // ցուցակում պահում ենք միայն առաջին MaxErrors-ը, բայց հաշվում ենք բոլորը
    if( _count <= MaxErrors )
        _errors.push_back({line, std::string{message}});
}
```

իսկ վերլուծիչի կողմից.

```cpp
// diagnostics.hxx
void advance() noexcept { _advanced = true; }
```

Դրոշն սկզբում `true` է, ուստի առաջին սխալը միշտ գրանցվում է։ Գրանցումից հետո
այն դառնում է `false`, և հաջորդ բոլոր հաղորդագրությունները լռվում են, մինչև
վերլուծիչը կլանի գոնե մեկ թոքեն։ Սա ուղիղ համարժեք է Վիրտի `p > errpos`
պայմանին, բայց փոխարինում է երկու հաշվիչ դաշտ մեկ բուլյան դրոշով։

`MaxErrors`-ը (8) համապատասխանում է Վիրտի `errcnt < 25` սահմանափակմանը։ Այն
պաշտպանում է օգտագործողին անիմաստ երկար ցուցակից. եթե ֆայլում այդքան սխալ կա,
ամենայն հավանականությամբ խնդիրը մեկն է՝ շատ վաղ սկզբում։

Ուշադրություն դարձրեք, որ `_count`-ը մեծանում է սահմանաքանակից հետո էլ։ Դա
Վիրտի `errcnt`-ի նույն դերն է. ցուցակը կտրված է, բայց իրական քանակը հայտնի է,
և օգտագործողը լռելյայն չի մոլորվում.

```
prog.bas:2: Սպասվում է հրաման, բայց հանդիպել է անհայտ նիշ '@'։
...
... և ևս 492 սխալ։
```

Ամեն սխալ պահվում է որպես.

```cpp
// Վերլուծության սխալ
struct SyntaxError {
    unsigned int line = 0; // տողի համարը
    std::string message;   // հաղորդագրությունը
};

std::ostream& operator<<(std::ostream& os, const SyntaxError& err);
```

#### Ո՞ւմ է պատկանում սխալների ցուցակը

`Diagnostics`-ը վերլուծիչի մաս չէ. այն ստեղծում է կանչողը և փոխանցում է
հղումով.

```cpp
Parser(Scanner& sc, Diagnostics& diag);
```

Այս ձևով նույն հավաքիչը կարող են կիսել կոմպիլյատորի բոլոր փուլերը՝ բառային
վերլուծիչը, շարահյուսականը և իմաստային ստուգումները, և օգտագործողը կստանա
սխալների մեկ միասնական ցուցակ։

#### Հաղորդագրության ձևակերպումը

Առանձին ուշադրության է արժանի `describe()` ֆունկցիան.

```cpp
std::string describe(const Lexeme& lex)
{
    switch( lex.kind ) {
        case Token::NewLine:    return "տողի ավարտ";
        case Token::Eof:        return "ֆայլի ավարտ";
        case Token::None:       return std::format("անհայտ նիշ '{}'", lex.value);
        case Token::Number:
        case Token::Text:
        case Token::Identifier: return std::format("'{}'", lex.value);
        default:                return std::format("'{}'", toString(lex.kind));
    }
}
```

Առանց դրա հաղորդագրության մեջ ուղղակի կդրվեր լեքսեմի `value`-ն, իսկ նոր տողի
լեքսեմի արժեքը հենց նոր տողի նիշն է — հաղորդագրությունը կկոտրվեր երկու մասի.

```
2: Սպասվում է THEN, բայց հանդիպել է
։
```

`describe()`-ի շնորհիվ ստացվում է.

```
2: Սպասվում է 'THEN', բայց հանդիպել է տողի ավարտ։
```

### 2.4. `match()` — բացակայող թոքենի «տեղադրումը»

Վիրտի մոտ թոքենի ստուգումը գրվում է հետևյալ ձևով.

```oberon
IF sym = expected THEN Get(sym) ELSE Mark("... expected") END
```

Ուշադրություն դարձրեք՝ `ELSE` ճյուղը միայն գրանցում է սխալը։ Ոչինչ չի
ընդհատվում, ոչ մի թոքեն չի կլանվում. վերլուծությունը շարունակվում է այնպես,
**կարծես բացակայող թոքենը տեղում էր**։ Այս հնարքը կոչվում է _թոքենի
տեղադրում_ (token insertion)։

Հենց սա է մեր `match()`-ը (տե՛ս 1.4)։ Արդյունքն այն է, որ մեկ բաց թողնված
թոքենն արժենում է ուղիղ մեկ սխալի հաղորդագրություն և ոչ մի կորցրած հանգույց։
Օրինակ՝

```basic
SUB Main
IF 1
LET a = 1
END IF
END SUB
```

Այստեղ `THEN`-ը բացակայում է։ `match(Token::Then)`-ը գրանցում է սխալը և
վերադառնում, իսկ `parseIfThen()`-ը շարունակում է կարդալ բլոկի մարմինը։
Արդյունքը՝ **մեկ սխալ և ամբողջությամբ վերականգնված `IF` հանգույց**.

```
2: Սպասվում է 'THEN', բայց հանդիպել է տողի ավարտ։

(basic-program (basic-subroutine "Main" '()
  (basic-sequence (basic-if (basic-number 1)
    (basic-sequence (basic-let (basic-variable "a") (basic-number 1)))))))
```

Բլոկների ավարտի համար կա առանձին օժանդակ ֆունկցիա.

```cpp
void Parser::parseBlockEnd(Token keyword)
{
    // 'END'-ը բացակայելիս բանալի բառը չենք էլ փնտրում, որպեսզի մեկ սխալի
    // համար երկու հաղորդագրություն չստացվի
    if( !lookahead.is(Token::End) ) {
        diagnostics.mark(lookahead.line, std::format("Սպասվում է 'END {}', բայց հանդիպել է {}։",
                         toString(keyword), describe(lookahead)));
        return;
    }

    advance();
    match(keyword);
}
```

### 2.5. `sync()` — համաժամեցման կետերը

Երբեմն «տեղադրումը» բավական չէ. եթե ընթացիկ թոքենն ընդհանրապես անտեղի է
(օրինակ՝ հրամանի փոխարեն `)` նշանն է), վերլուծիչին պետք է հնարավորություն տալ
_վերագտնել իրեն_։ Դրա համար Վիրտը հիմնական կառուցվածքների սկզբում դնում է
**համաժամեցման կետեր**. այնտեղ բաց են թողնվում բոլոր այն թոքենները, որոնցով
տվյալ կառուցվածքը սկսվել չի կարող.

```cpp
void Parser::sync(const std::set<Token>& stops, std::string_view message)
{
    if( stops.contains(lookahead.kind) )
        return;

    diagnostics.mark(lookahead.line, message);

    while( !stops.contains(lookahead.kind) )
        advance();
}
```

Այս իրականացման մեջ երեք համաժամեցման կետ կա՝ ըստ երեք մակարդակի.

| Կետը | Բազմությունը | Ի՞նչ է որոնում |
|------|--------------|----------------|
| `parseProgram()` | `SubroutineSync` = `{SUB, Eof}` | հաջորդ ենթածրագիրը |
| `parseSequence()` | `StatementSync` = `NewLine ∪ FirstStat ∪ FollowStat` | հաջորդ հրամանը |
| `parseFactor()` | `ExprSync` | հաջորդ արտահայտությունը կամ փակող նշանը |

`ExprSync`-ը հատուկ ուշադրության է արժանի.

```cpp
// Արտահայտության մակարդակի համաժամեցման կետերը
//
// Բացի արտահայտություն սկսող թոքեններից՝ պարունակում է նաև փակող
// թոքենները, որպեսզի սխալ արտահայտությունը չկլանի իրեն շրջապատող
// կառուցվածքը։
const std::set<Token> ExprSync = {
    Token::True, Token::False, Token::Number, Token::Text, Token::Identifier,
    Token::Sub, Token::Not, Token::LeftPar,
    Token::RightPar, Token::RightBrack, Token::Comma,
    Token::NewLine, Token::End, Token::ElseIf, Token::Else, Token::Subroutine, Token::Eof
};
```

Եթե բազմության մեջ չլինեին `)`, `]` և `,` թոքենները, ապա `DIM arr[]` սխալի
դեպքում վերլուծիչը կկլաներ փակող `]`-ը և կշարունակեր հեռու գնալ՝ ոչնչացնելով
շրջապատող `DIM` հրամանը։

### 2.6. Չեզոք հանգույցը

Առանց բացառությունների աշխատելիս առաջանում է մի հարց. ի՞նչ վերադարձնի
`parseFactor()`-ը, եթե արտահայտություն այդպես էլ չգտավ։ Պատասխանը՝ **չեզոք
հանգույց**.

```cpp
Expression::Ptr Parser::parseFactor()
{
    // sync
    if( !FirstExpr.contains(lookahead.kind) ) {
        diagnostics.mark(lookahead.line, std::format("Սպասվում է արտահայտություն, բայց հանդիպել է {}։", describe(lookahead)));

        while( !ExprSync.contains(lookahead.kind) )
            advance();
    }

    if( lookahead.is(Token::True, Token::False) )  return parseTrueOrFalse();
    if( lookahead.is(Token::Number) )              return parseNumber();
    if( lookahead.is(Token::Text) )                return parseText();
    if( lookahead.is(Token::Identifier) )          return parseIdentOrApply();
    if( lookahead.is(Token::LeftPar) )             return parseGrouped();

    // արտահայտություն այդպես էլ չգտնվեց. վերադարձնում ենք չեզոք հանգույց,
    // որպեսզի կանչողները շարունակեն սովորական ձևով
    return node<Number>(0.0, lookahead.line);
}
```

Այս `Number(0.0)` հանգույցը սխալ արժեք է, բայց դա նշանակություն չունի, քանի որ
սխալների առկայության դեպքում ծառն ընդհանրապես չի օգտագործվում (տե՛ս 2.8)։
Փոխարենը շահում ենք գլխավորը՝ բոլոր կանչող ֆունկցիաները ստանում են վավեր
ցուցիչ և շարունակում աշխատել առանց հատուկ դեպքերի ստուգման։

### 2.7. Ինչու է վերլուծությունը միշտ ավարտվում

Սխալների վերականգնման ամենավտանգավոր սխալը **անվերջ ցիկլն** է. վերականգնման
ցիկլը կրկնվում է՝ առանց ոչ մի թոքեն կլանելու։ Այս իրականացման մեջ ավարտվելը
ապահովված է կառուցվածքով։

Դիտարկենք `parseSequence()`-ի ցիկլը։ Ամեն կրկնության մեջ հնարավոր է երեք դեպք.

1. `lookahead`-ը `NewLine` է → `parseNewLines()`-ը կլանում է առնվազն մեկ թոքեն։
2. `lookahead`-ը `StatementSync`-ում չէ → `sync()`-ը կլանում է առնվազն մեկը։
3. `lookahead`-ը `FirstStat`-ում է → `parseOneStatement()`-ը կլանում է առնվազն
   սկզբնական բանալի բառը։

Այլ դեպք չկա, քանի որ

```
StatementSync = {NewLine} ∪ FirstStat ∪ FollowStat
```

իսկ `FollowStat`-ի թոքենները ցիկլից դուրս են բերում։ Հետևաբար ամեն կրկնություն
կլանում է առնվազն մեկ թոքեն, իսկ թոքենների քանակը վերջավոր է։ Նույն
դատողությունը կիրառելի է `parseProgram()`-ի և `parseFactor()`-ի համար։

Այս պնդումը ստուգված է նաև գործնականում՝ 4000 պատահական թոքենային
հաջորդականությամբ, որոնցից ոչ մեկը չի հանգեցրել կախվելու։

### 2.8. Թերի ծառը հաջորդ փուլերին չի փոխանցվում

Վերականգնումից հետո ստացված ծառը **թերի** է. այնտեղ կարող են լինել անանուն
ենթածրագրեր, չեզոք հանգույցներ, բացակայող ճյուղեր։ Այն պիտանի է սխալների
հաղորդման ու գործիքների (խմբագրիչի ընդգծումներ և այլն) համար, բայց ոչ
կոդագեներացիայի։

Ուստի `parse()`-ի արդյունքն օգտագործելուց առաջ կանչողը **պարտավոր է** ստուգել
սխալների ցուցակը։ Այս պայմանագիրը գրված է վերնագրում.

```cpp
// Ուշադրություն. սխալի դեպքում վերլուծությունը չի ընդհատվում, և վերադարձված
// ծառը թերի է լինում։ Կանչողը պարտավոր է նախ ստուգել Diagnostics-ը և
// միայն սխալների բացակայության դեպքում ծառը փոխանցել հաջորդ փուլերին։
Program::Ptr parse();
```

և պահպանվում է [src/compiler.cxx](src/compiler.cxx)-ում.

```cpp
Diagnostics diagnostics;
Parser parser{scanner, diagnostics};
auto program = parser.parse();

// սխալների առկայության դեպքում ծառը թերի է, ուստի հաջորդ փուլերին
// չի փոխանցվում
if( !diagnostics.errors().empty() ) {
    for( const auto& error : diagnostics.errors() )
        std::cerr << source.string() << ":" << error << std::endl;

    if( diagnostics.count() > diagnostics.errors().size() )
        std::cerr << std::format("... և ևս {} սխալ։",
                diagnostics.count() - diagnostics.errors().size()) << std::endl;

    return false;
}
```

### 2.9. Օրինակներ

| Մուտք | Սխալներ | Վերականգնման արդյունքը |
|-------|---------|------------------------|
| `IF 1` (առանց `THEN`) | 1 | `IF`-ը վերականգնվում է ամբողջությամբ |
| `IF 1 THEN ... END` (առանց `IF`) | 1 | `IF`-ը և հաջորդող հրամանը պահպանվում են |
| `LET x = (1` | 1 | `LET`-ը և հաջորդող հրամանը պահպանվում են |
| `SUB Main` առանց `END SUB` | 1 | երկու ենթածրագրերն էլ պահպանվում են |
| `LET x = *` | 1 | չեզոք հանգույց, հաջորդ հրամանը՝ նորմալ |
| `FOO` հրամանի փոխարեն | 1 | հրամանը բաց է թողնվում |
| `LET = 1` | 1 | անանուն փոփոխականով `LET` |
| `SUB` առանց անվան | 1 | անանուն ենթածրագիր՝ ամբողջական մարմնով |

Այս բոլոր դեպքերը ծածկված են թեսթերով՝ [tests/parser_test.cxx](tests/parser_test.cxx),
`[recovery]` պիտակի տակ։

#### Եղանակի սահմանափակումը

Պարզ եղանակն ունի իր գինը։ Դիտարկենք չփակված ներքին բլոկը.

```basic
SUB Main
WHILE 1
LET x = 1
END SUB
```

Այստեղ `END SUB`-ը կլանվում է `WHILE`-ի ավարտի փոխարեն, ուստի ենթածրագիրն
անավարտ է մնում, և ստացվում է երեք սխալ՝ մեկի փոխարեն։ Այս խնդիրը հատուկ է
նաև Վիրտի Oberon-0 կոմպիլյատորին։ Այն կարելի է լուծել երկու թոքեն առաջ
նայելով (այսինքն՝ ստուգելով, թե `END`-ին ի՛նչ բանալի բառ է հաջորդում), բայց
դա խախտում է LL(1) սահմանափակումը և բարդացնում է `advance()`-ը՝ հանուն
համեմատաբար հազվադեպ դեպքի։

### 2.10. Այլ եղանակների հետ համեմատություն

| Եղանակ | Գաղափարը | Դիտարկում |
|--------|----------|-----------|
| **Խուճապի ռեժիմ բացառություններով** (panic mode) | սխալի դեպքում դեն նետել բացառություն, բռնել վերականգնման կետում, բաց թողնել մինչև համաժամեցնող թոքեն | ամենատարածվածն է ժամանակակից ձեռագիր վերլուծիչներում, օրինակ՝ *Crafting Interpreters*-ում։ Պահանջում է `panicMode` դրոշ, `try`/`catch` բլոկներ և առաջընթացի առանձին երաշխիք |
| **Վիրտի եղանակ** (այս իրականացումը) | `Mark` + `match` + համաժամեցման կետեր, առանց բացառությունների | ամենապարզն է. կառավարման հոսքը միշտ գծային է։ Գինը՝ բարդ դեպքերում մեկ-երկու ավելորդ հաղորդագրություն |
| **FOLLOW բազմությունների փոխանցում** | ամեն վերլուծող ֆունկցիա ստանում է իրեն հաջորդող թոքենների բազմությունը որպես պարամետր | ավելի ճշգրիտ վերականգնում, բայց ամեն ֆունկցիա ձեռք է բերում լրացուցիչ պարամետր |
| **Սխալի կանոններ** (error productions) | քերականության մեջ բացահայտ նկարագրվում են տարածված սխալները | թույլ է տալիս շատ լավ հաղորդագրություններ, բայց քերականությունը մեծանում է |
| **Ամենափոքր ուղղում** (minimum-distance repair) | գտնել տեղադրումների/ջնջումների նվազագույն հաջորդականությունը, որը մուտքը դարձնում է վավեր | տեսականորեն գեղեցիկ, գործնականում՝ թանկ |

---

## 3. Գրականություն և հղումներ

### Հիմնական աղբյուրը

- **Niklaus Wirth, _Compiler Construction_** (Addison-Wesley, 1996) — այս
  իրականացման հիմքը։ Ամբողջ գիրքը հեղինակի կայքում հասանելի է անվճար.
  - [Գրքի էջը ETH Zürich-ում](https://people.inf.ethz.ch/wirth/CompilerConstruction/index.html)
  - [Գլուխներ 1–8 (PDF)](https://people.inf.ethz.ch/wirth/CompilerConstruction/CompilerConstruction1.pdf)
  - [Գլուխներ 9–16 (PDF)](https://people.inf.ethz.ch/wirth/CompilerConstruction/CompilerConstruction2.pdf)

  Հատկապես կարևոր են՝ **գլուխ 4** (ռեկուրսիվ իջնող վերլուծություն, LL(1)
  պայմանները), **գլուխ 7** (Oberon-0-ի վերլուծիչը) և հատկապես **7.3 բաժինը՝
  «Coping with syntactic errors»**։

- **Project Oberon** — Վիրտի կոմպիլյատորի իրական սկզբնական կոդը, որտեղ կարելի
  է տեսնել `Mark`-ը և համաժամեցման կետերն «աշխատանքի մեջ».
  - [Project Oberon-ի էջը](https://people.inf.ethz.ch/wirth/ProjectOberon/index.html)
  - `ORS.Mod` — բառային վերլուծիչը և `Mark` պրոցեդուրան
  - `ORP.Mod` — շարահյուսական վերլուծիչը
  - [lboasso/oberon0](https://github.com/lboasso/oberon0) — գրքի կոդի հարմարեցված տարբերակը GitHub-ում

### Լրացուցիչ ընթերցանություն

- **Robert Nystrom, _Crafting Interpreters_** — ժամանակակից, շատ մատչելի
  նկարագրություն. ռեկուրսիվ իջնող վերլուծիչ և խուճապի ռեժիմ բացառություններով
  (այսինքն՝ այն այլընտրանքը, որը նկարագրված է 2.10 բաժնում)։ Անվճար է առցանց.
  - [Parsing Expressions](https://craftinginterpreters.com/parsing-expressions.html)
  - [Synchronizing a recursive descent parser](https://craftinginterpreters.com/parsing-expressions.html#synchronizing-a-recursive-descent-parser)

- **Alfred Aho, Monica Lam, Ravi Sethi, Jeffrey Ullman, _Compilers:
  Principles, Techniques, and Tools_** («վիշապի գիրքը»), 2-րդ հրատարակություն
  — 4.1.3 բաժինը («Syntax Error Handling») և 4.4.5-ը («Error Recovery in
  Predictive Parsing») տալիս են սխալների վերականգնման եղանակների
  համակարգված դասակարգում՝ FIRST/FOLLOW բազմությունների լեզվով։

- **Dick Grune, Ceriel J.H. Jacobs, _Parsing Techniques — A Practical
  Guide_**, 2-րդ հրատարակություն (Springer, 2008) — վերլուծության եղանակների
  ամենամանրամասն ձեռնարկը. գլուխ 16-ը ամբողջությամբ նվիրված է սխալների
  մշակմանը։

- **Jack Crenshaw, _Let's Build a Compiler_** — քայլ առ քայլ ձեռագիր
  վերլուծիչի կառուցումը՝ շատ մատչելի ոճով.
  [compilers.iecc.com/crenshaw](https://compilers.iecc.com/crenshaw/)

- **Niklaus Wirth, _Algorithms + Data Structures = Programs_** (1976) —
  գրքի վերջին գլխում ներկայացված PL/0 կոմպիլյատորը նույն եղանակի ավելի վաղ և
  ավելի պարզ տարբերակն է. այնտեղ վերականգնումն արվում է `test(s1, s2, n)`
  պրոցեդուրայով, որը հենց այս փաստաթղթի `sync()`-ի նախատիպն է։

### Կապված փաստաթղթեր այս պահոցում

- [book/grammar.md](book/grammar.md) — լեզվի ամբողջական քերականությունը
- [book/chapter-01.md](book/chapter-01.md) — քերականության բացատրությունը
- [book/chapter-90.md](book/chapter-90.md) — գրականության ցանկը
