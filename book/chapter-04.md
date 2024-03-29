# Շարահյուսական վերլուծություն

Մեր կոմպիլյատորի իրականացման համատեքստում _շարահյուսական վերլուծությունը_ ներառում է երեք ինքնուրույն, բայց միմյանց հետ սերտորեն հապված գործողություններ։ Դրանք են.

* _բառային վերլուծությունը_, որը ծրագրի տեքստում առանցձնացնում է BASIC-IR լեզվի բառային (լեքսիկական) միավորները` ծառայողական բառերը, սիմվոլները և այլն,
* _շարահյուսական վերլուծությունը_, որը բառային վերլուծիչի կազմած բառերից կառուցում է BASIC-IR լեզվի քերականական կանոններին համապատասխան «նախադասություններ»,
* _տիպերի ստուգումը_, որը ստուգում է շարահյուսական վերլուծիչի կառուցած նախադասությունների՝ տիպերի տեսակետից ճիշտ լինելը։

Հետագա երեք գլուխներում առանձին-առանձին ու մանրամասնորեն կխոսենք ոչ միայն նշված գործողությունների նշանակության և կոմպիլյատորի իրականացման մեջ դրանց տեղի մասին, այլև կդիտարկենք իրականացման հարցերը։

## Բառային վերլուծություն

Բառային վերլուծիչը (կամ scanner, lexical analyzer) նիշ-առ-նիշ կարդում է ծրագրի տեքստը և այդ կարդացած նիշերից կազմում է իմաստալից բառային միավորներ։ Օրինակ, եթե տրված է ծրագրի մի այսպիսի տող.

```basic
INPUT "Name: ", name$
```

ապա բառային վերլուծիչը կարդում է նիշերի հետևյալ շարքը. «`I`», «`N`», «`P`», «`U`», «`T`», «`␣`», «`"`», «`N`», «`a`», «`m`», «`e`», «`:`», «`␣`», «`"`», «`,`», «`␣`», «`n`», «`a`», «`m`», «`e`», «`$`»։ Այնուհետև դրանցից կազմում է BASIC-IR լեզվի `INPUT`, `Name: `, `,` և `name$` բառային միավորները։ 

Բացի բառերն միավորներն առանձնացնելը բառային վերլուծիչը նաև տեսակավորում է դրանք՝ յուրաքանչյուրին վերագրելով իր տեսակի պիտակը։ Վերը բերված օրինակի բառերը ստանում են այսպիսի պիտակներ.

| Բառ      | Պիտակ              | Նշանակություն            |
| :------: | :-----------------: | :-----------------------: |
| `INPUT`  | `Token::Input`      | `INPUT` ծառայողական բառը |
| `Name:␣` | `Token::Text`       | տեքստային լիտերալ       |
| `,`      | `Token::Comma`      | ստորակետ                 |
| `name$`  | `Token::Identifier` | իդենտիֆիկատոր           |

Այսպիսով. _բառային վերլուծիչն իր մուտքին ստանում է նիշերի հաջորդականություն, իսկ ելքում տալիս է պիտակավորված բառերի հաջորդականություն_։ 


### Նիշերի հոսքից բառերի կառուցելը

Հիմա տեսնենք թե ինչ եղանակով են նիշերի հոսքից կառուցվում տվյալ լեզվի բառապաշարին համապատասխանող բառային միավորները։ Բառային վերլուծիչներ կառուցելու ամենատարածված եղանակը  վերջավոր ավտոմատի մոդելի կառուցումն է։ 

### `Lexeme` դասը

Պիտակավորված բառը ներկայացնելու համար սահմանել ենք `Lexeme` դասը։ Այն միավորումը է բառի պիտակը (`kind`), բառը (`value`) և տողի համարը (`line`), որտեղ հանդիպել է բառը։ (Տողի համարն անհրաժեշտ է տխալների մասին հաղորդագրություններ արտածելիս ծրագրավորողին ավելի կոնկրետ տեղեկություն տրամադրելու համար։)

```C++
class Lexeme {
public:
    Token kind = Token::None; // պիտակը
    std::string value = "";   // բառը (լեքսեմը)
    unsigned int line = 0;    // տողի համարը
  
    // ...
};
```

BASIC-IR լեզվում նախատեսված բոլոր պիտակները ներկայացրել ենք `Token` թվարկմամբ։ Դրանում արժեքներ են նախատեսված _ծառայողական բառերը_, _տեքստային_ ու _թվային_ լիտերալները, _փոփոխականները_, _թվաբանական_, _տրամաբանական_ ու _համեմատման_ գործողությունները, ինչպես նաև լեզվի այլ հատուկ սիմվոլները պիտակավորելու համար։

```C++
enum class Token : int {
    None, // ոչինչ

    Number,     // թվային հաստատուն
    Text,       // տեքստային հաստատուն
    Identifier, // իդենտիֆիկատոր

    // ծառայողական բառեր
    Subroutine, // SUB
    Input,      // INPUT
    Print,      // PRINT
    Let,        // LET
    If,         // IF
    Then,       // THEN
    ElseIf,     // ELSEIF
    Else,       // ELSE
    While,      // WHILE
    For,        // FOR
    To,         // TO
    Step,       // STEP
    Call,       // CALL
    End,        // END

    NewLine, // նոր տողի նիշ

    Eq, // =
    Ne, // <>
    Lt, // <
    Le, // <=
    Gt, // >
    Ge, // >=

    LeftPar,  // (
    RightPar, // )
    Comma,    // ,

    Add, // +
    Sub, // -
    Amp, // &
    Or,  // OR
    Mul, // *
    Div, // /
    Mod, // MOD
    And, // AND
    Pow, // ^
    Not, // NOT

    Eof // ֆայլի վերջը
};
```

### `Scanner` դասը

Բառային վերլուծիչն իրականացված է `Scanner` դասով։ Այն ստանում է վերլուծվելիք ֆայլի ճանապարհը, և `operator>>` գործողությամբ վերադարձնում է ֆայլից կարդացած հերթական լեքսեմը։

```C++
class Scanner {
private:
    // ծառայողական բառերի ցուցակ
    static std::map<const std::string,Token> keywords;

public:
    Scanner( const std::string& filename );
    ~Scanner();

    Scanner& operator>>( Lexeme& lex );

private:
    // կառուցում է հերթական լեքսեմը
    bool next( Lexeme& lex );
    // կարդում է թվային լիտերալ
    bool scanNumber( Lexeme& lex );
    // կարդում է տեքստային լիտերալ
    bool scanText( Lexeme& lex );
    // կարդում է իդենտիֆիկատոր
    bool scanIdentifier( Lexeme& lex );

private:
    // նիշեր կարդալու հոսքը
    std::ifstream source;

    // հոսքից կարդացած հերթական նիշը
    char ch = '\0';
    // ֆայլի ընթացիկ տողը
    unsigned int line = 1;
};
```

Բուն բառային վերլուծությունը կատարվում է `Scanner::next(Lexeme&)` մեթոդում։ Սա ֆայլից կարդում է հերթական լեքսեմն ու իր հղումային արգումենտի միջոցով վերադարձնում է կանչողին։

## Շարահյուսական վերլուծություն

## Տիպերի ստուգում և դուրսբերում

Շարահյուսական վերլուծության հաջող ավարտին կառուցվում է լեզվի քերականությանը համապատասխան վերլուծության ծառ։ Քանի որ լեզվի քերականությունը կառուցված է առանց տիպերի մասին ինֆորմացիան հաշվի առնելու, ստացված վերլուծության ծառը նույնպես ...։ Վերլուծության ծառը կոդի գեներատորին փոխանցելուց առաջ պետք է անցնել բոլոր հանգույցներով և ամեն մի կետում ստուգել տիպերի կոռեկտությունը։ Օրինակ, բինար գործողության հանգույցում պետք է ստուգել, որ գումարման գործողության օպերանդներն ունենան թվային տիպ, իսկ տողերի կցման գործողության օպերանդները անպայմանորեն ունենան տեքստային տիպ։

Մեր նպատակն է ծառի ամեն մի հանգույցում ստուգել տիպերի կոռեկտությունը և, անհրաժեշտության դեպքում, լրացել պակասող տվյալները։ Նույն բինար գործողության օրինակով. եթե հանգույցի երկու օպերանդներն էլ տեքստային են, ապա պետք է հանգույցի տիպը նշել որպես `TEXT` միայն ան դեպքում, երբ գործողությունը տեքստերի միակցումն է։

Թվային ու տեքստային հաստատունների տիպը հայտնի է հենց վերլուծության ժամանակ։ Փոփոխականի տիպը որոշվում է նրա անվան կառուցվածքով։

Ունար գործողությունների տիպը կարող է լինել միայն թվային։

Բինար գործողությունների տիպը դուրս է բերվում օպերանդների տիպերից ու գործողության կոդից։

Ֆունկցիայի կիրառման `Apply` հանգույցի տիպը որոշվում է կանչվող ֆունկցիայի տիպով։ Այս դեպքում պետք է ստուգել նաև ֆունկցիայի պարամետրերի ու կանչի արգումենտների տիպերի համապատասխանությունը։

Վերագրման հրամանում պետք է ստուգել փոփոխականի ու վերագրվող արժեքի տիպերի համապատասխանությունը։

Ճյուղավորման հրամանում պետք է ստուգել որ պայմանի արտահայտության տիպը թվային լինի։

Պարամետրով ցիկլի հրամանում պարամետրը, դրա սկզբնական ու վերջնական արժեքի արտահայտությունները պետք լինեն թվային տիպի։ Պարամետրի քայլի թվային տիպը ապահովված է քերականությամբ։

Պայմանով ցիկլի պայմանի տիպը պետք է լինի թվային։

Պրոցեդուրայի կանչի դեպքում պետք է համապատասխանեն ենթածրագրի պարամետրերի տիպերն ու կանչի արգումենտների տիպերը։

