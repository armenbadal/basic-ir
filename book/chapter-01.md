# BASIC-IR լեզուն

Այս գրքում իրականացնում ենք BASIC[^1] լեզվի մի տարատեսակ, որին տվել ենք BASIC-IR անունը։ IR մասնիկը հուշում է, որ իրականացվող կոմպիլյատորը BASIC ծրագիրը թարգմանում է LLVM համակարգի IR[^2] ներկայացման։ Լեզուն ունի երկու ներդրված տիպ՝ _իրական_ (`REAL`) և _տեքստային_ (`TEXT`)։ Իրական արժեքների հետ կարելի է կատարել թվաբանական, տրամաբանական և համեմատման գործողություններ։ Տեքստային արժեքների հետ կարելի է կատարել միակցման (concatenation) գործողությունը։ Փոփոխականի տիպը որոշվում է նրա անվան կառուցվածքով. եթե իդենտիֆիկատորն ավարտվում է `$` նիշով, օրինակ, `a$` կամ `se$`, ապա այն `TEXT` տիպի է, հակառակ դեպքում՝ `REAL`: Տիպերը խստորեն ստուգվում են վերլուծության ժամանակ, իսկ անհամապատասխանությունների դեպքում կոմպիլյատորը հայտնում է սխալի մասին։ Փոփոխականը սահմանվում է և արժեք է ստանում _վերագրման_ `LET` հրամանով։ Օրինակ.

```basic
LET a = 3.14       ' սահմանվում է a փոփոխակնը՝ 3,14 սկզբնական արժեքով
LET b$ = "Yes"     ' սահմանվում է b$ փոփոխակնը՝ «Yes» սկզբնական արժեքով
LET a = a / 2      ' a փոփոխականի արժեքը կիսվում է
LET b$ = b$ & "?"  ' b$ փոփոխականը ստանում է «Yes?» արժեքը
```

Ծրագրում որոշումները կատարվում են _ճյուղավորման_ `IF` հրամանով։ Օրինակ.

```basic
' եթե a > b, ապա ...
IF a > b THEN
  ' արտածել a-ն
  PRINT a
END IF
```

Կրկնությունները կազմակերպվում են _կրկնման_ `FOR` ու `WHILE` հրամաններով։ Առաջինը դասական _պարամետրով_ ցիկլ է, իսկ երկրորդը՝ _պայմանով_։ Օրինակ, եթե պահանջվում է նվազման կարգով արտածել 1..100 միջակայքի զույգ թվերը, ապա կարելի է `FOR` ցիկլի կիրառությամբ գրել.

```basic
FOR i = 100 TO 1 STEP -2
  PRINT i
END FOR
```

Նույնը՝ `WHILE` ցիկլի կիրառությամբ.

```basic
LET i = 100
WHILE i > 1
  PRINT i
END WHILE
```

Որպես ծրագրի կազմակերպման միավոր BASIC-IR լեզուն թույլատրում է սահմանել _ենթածրագրեր_ (subroutine)։ BASIC-IR ծրագրում ենթածրագրերը կարող են կատարել և՛ պրոցեդուրայի, և՛ ֆունկցիայի դեր։ Օրինակ, `Area` պրոցեդուրան արտածում է տրված `r` շառավղով շրջանի մակերեսը։

```basic
SUB Area(r)
  LET ca = 3.1415 * r^2
  PRINT ca
END SUB
```

Այս ենթածրագիրը կարելի է _կանչել_ `CALL` հրամանով: Այսպես.

```basic
CALL Area 2.3
```

`Area` ենթածրագիրը կարող է նաև _վերադարձնել_ շրջանի մակերեսը։ Դրա համար պետք է պարզապես հաշվարկված արժեքը վերագրել ենթածրագրի անունին։

````basic
SUB Area(r)
  LET Area = 3.1415 * r^2
END SUB
````

Այս եղանակով սահմանված ենթածրագրերը կարող են կատարել _ֆունկցիայի_ դեր և կարող են օգտագործվել արտահայտությունների մեջ։ Օրինակ.

```basic
WHILE Area(k) > e0
  LET k = k - 0.2
  ' ...
END WHILE
```

BASIC-IR լեզվով գրված ծրագիրը, որ նախընտրելի է պահել `*.bas` վերջավորությամբ ֆայլում, ենթածրագրերի սահմանումների հաջորդականություն է։ Այդ ֆայլն անվանում ենք _կոմպիլյացիայի միավոր_։

## BASIC-IR քերականությունը

BASIC-IR լեզվի նկարագրությունը սկսենք ամենախոշոր միավորներից՝ ծրագրից ու ենթածրագրից։ Այս դեպքում մեր ամեն մի քայլում սահմանելու ենք նախորդ քերականական հավասարման մեջ օգտագործված բայց դեռ չսահմանված ոչ տերմինալային սիմվոլները։ Կարելի է, իհարկե, լեզվի քերականությունը սահմանել «մանրերից դեպի խոշորները» սկզբունքով։ Այդ դեպքում էլ ամեն մի քայլում քերականական հավասարումը կառուցվում է նախորդ քայլերում արդեն սահմանված ոչ տերմինալներից ու տերմինալներից։

### Ծրագիր և ենթածրագիր

BASIC-IR լեզվով գրված _ծրագիրը_ ենթածրագրերի սահմանումների հաջորդականություն է՝ գրառված `.bas` վերջավորությամբ ֆայլում։ 

```
Program = [NewLines] { Subroutine NewLines }.
```

Քանի որ ծրագիրը պարունակող ֆայլը կարող է սկսվել դատարկ տողերով (կամ մեկնաբանություններով), `Program`-ը սահմանող քերականական հավասարումը սկսել ենք `NewLines` ոչ պարտադիր տարրով։ `NewLines`-ն էլ սահմանվում է որպես մեկ կամ ավելի նոր տողի անցման նիշեր։

```
NewLines = NL { NL }.
```

Այնուհետև `{` և `}` գործողության օգնությամբ նշում ենք, որ ծրագրում կարող են սահմանվել զրո կամ ավելի ենթածրագրեր, դրանցից յուրաքանչյուրի սահմանումը պետք է ավարտվի գոնե մեկ նոր տողի նիշով։

_Ենթածրագրի_ սահմանումը սկսվում է `SUB` ծառայողական բառով, որին հաջորդում է _անունը_ և _պարամետրերի ցուցակը_։ Այնուհետև՝ ենթածրագրրի մարմինը ձևավորող հրամանների հաջորդականությունը։ Եվ վերջում՝ `END` և `SUB` ծառայողական բառերը։

```
Subroutine = 'SUB' Ident ['(' IdentList ')'] Statements 'END' 'SUB'.
```

Ենթածրագրի սահմանման մեջ պարամետրերի ցուցակը պարտադիր չէ։ Եթե պարամետրերը բացակայում են, ապա պետք է բաց թողնել նաև դրանք պարփակող `(` և `)` փախագծերը։ Օրինակ.

```basic
SUB Hello
  PRINT "Hello, World!"
END SUB
```

Իդենտիֆիկատորների ցուցակը կազմվում է ստորակետով իրարից բաժանված մեկ կամ ավելի իդենտիֆիկատորներից։

```
IdentList = IDENT { ',' IDENT }.
```

Հրամանների `Statements` հաջորդականությունը անպայման սկսվում է նոր տողից, և ամեն մի հրամանն էլ իր հերթին ավարտվում է գոնե մեկ նոր տողի անցման նիշով։ 

```
Statements = NewLines { (Let | Input | Print | If | While | For | Call) NewLines }.
```

### Ղեկավարող կառուցվածքներ

Ծրագրային միավորների հաջորդ խմբում _ղեկավարող կառուցվածքներն_ են (կամ _հրամաննները_)։

Փոփոխականին նոր արժեք է տրվում _վերագրման_ հրամանով։ Այն սկսվում է `LET` ծառայողական բառով, որին հաջորդում են փոփոխականի անունը, `=` նշանը և մի արտահայտություն, որի արժեքը վերագրվում է փոփոխականին։ Սա նաև փոփոխականի _սահմանման_ հրամանն է։

```
Let = 'LET' IDENT '=' Expression.
```

_Տվյալների ներմուծման_ հրամանը սկսվում է `INPUT` բառով, որին հաջորդում է ներմուծման հրավերքի ոչ պարդադիր տողը և ներմուծվող փոփոխականը։ Հրավերքի տեքստն ու փոփոխականն իրարից անջատվում են ստորակետով։

```
Input = 'INPUT' [TEXT ','] IDENT.
```

_Տվյալների արտածման_ հրամանը սկսվում է `PRINT` բառով, որին հաջորդում է արտածվող արտահայտությունը։

```
Print = 'PRINT' Expression.
```

_Ճյուղավորման_ հրամանը սկսվում է `IF` ծառայողական բառով, որին նույն տողում հաջորդում է _պայմանի_ արտահայտությունը, ապա `THEN` ծառայողական բառը։ Ապա հաջորդում են այն հրամանները, որոնք պետք է կատարվեն պայմանի ճշմարիտ լինելու դեպքում։ `ELSEIF` ծառայողական բառով կարելի է նշել նոր պայման ու այդպիսով կազմել պայմանների շղթա։ Եթե այդ շղթայի պայմաններից և ոչ մեկը ճշմարիտ արժեք չի վերադարձնում, ապա կատարվում է `ELSE` ծառայողական բառից հետո գրված բլոկը (եթե այն առկա է)։

```
If = 'IF' Expression 'THEN' Statements
      { 'ELSEIF' Expression 'THEN' Statements }
      ['ELSE' Statements] 'END' 'IF'.
```

_Պայմանով ցիկլի_ հրամանը սկսվում է `WHILE` ծառայողական բառով, որին հաջորդում է պայմանի արտահայտությունը։ Հաջորդ տողից թվարկվում են այն հրամանները, որոնք կազմում են ցիկլի մարմինը։ Այս հրամանն ավարտվում է `END` և `WHILE` ծառայողական բառերով։

```
While = 'WHILE' Expression Statements 'END' 'WHILE'.
```

_Հաշվիչով ցիկլի_ հրամանը սկսվում է `FOR` ծառայողական բառով, որին հաջորդում է հաշվիչի արժեքավորման հրամանը։ `TO` ծառայողական բառից հետո գրվում է հաշվիչի վերին սահմանը ցույց տվող արտահայտությունը, իսկ `STEP` բառից հետո՝ հաշվիչի քայլը։ Հրամանն ավարտվում է `END` և `FOR` բառերով։

```
For = 'FOR' IDENT '=' Expression 'TO' Expression ['STEP' Expression]
      Statements 'END' 'FOR'.
```

_Պրոցեդուրայի կանչի_ հրամանը սկսվում է `CALL` ծառայողական բառով։ Դրան հետևում են կանչվող ենթածրագրի անունը և կանչի արգումենտները՝ ստորակետով իրարից անջատված արտահայտությունների ցուցակ։

```
Call = 'CALL' IDENT [Expression { ',' Expression }].
```


### Արտահայտություններ

Քերականության `Factor` կանոնը սահմանում է, թե ինչպիսին կարող է լինել ամենապարզ արտահայտությունը։

```
Factor = 'Ident'   (* փոփոխական *)
       | 'Integer' (* հաստատուն ամբողջ թիվ *)
       | 'Double'  (* հատատուն իրական թիվ *)
       | 'True'    (* բուլյան ՃՇՄԱՐԻՏ արժեքը *)
       | 'False'.  (* բուլյան ԿԵՂԾ արժեքը *)
```

Պարզագույն արտահայտություն է նաև ֆունկցիայի կանչը։ Օրինակ, վերը սահմանված `Gcd` ֆունկցիան արտահայտության մեջ կարող է օգտագործվել այսպես․

```basic
Dim r As Integer
r = Gcd(13, 572)
```

Ֆունկցիայի կանչը սկսվում է կանչվող ֆունկցիայի անունը ցույց տվող իդենտիֆիկատորով։ Ֆունկցիայի անունին հետևում են `(` և `)` փակագծերում վերցրած ու ստորակետով իրարից անջատված արգումենտները։

```
Factor = 'Ident' '(' [Disjunction { ',' Disjunction }] ')'.
```

Պարզ արտահայտություններ կարելի է համարել նաև _ունար_ գործողությունները՝ `-` \(բացասում\) և `Not` \(ժխտում\), ինչպես նաև խմբավորման փակագծերը՝ `()`։

```
Factor = '(' Disjunction ')'
       | '-' Factor
       | 'Not' Factor.
```

Խմբավորման փակագծերի քերականությունը նկարագրող կանոնում `Disjunction` ոչ֊տերմինալային սիմվոլի օգտագործումը ցույց է տալիս, որ փակագծերի օգնությամբ կարելի է «բարձրացնել» ենթաարտահայտությունների հաշվման նախապատվությունը \(ինչպես, ի դեպ, արվում է համարյա բոլոր ծրագրավորման լեզուներում\)։ \(??\)

Արտահայտությունների սահմանման հաջորդ մակարդակում թիվն աստիճան բարձրացնելու `^` գործողությունն է։ Սա աջ֊ասոցեատիվ գործողություն է։ \(??\)

```
Power = Factor ['^' Power].
```

Թվաբանական գործողությունները բաժանված են երկու խմբի՝ _բազմապատկող_ և _գումարող_։ Առաջինում բազմապատկման, բաժանման և մնացորդի որոշման գործողություններն են, իսկ երկրորդում՝ գումարման ու հանման։

```
      Addition = Multiplication { ('+'|'-') Multiplication }.
Multiplication = Power { ('*'|'/'|'\\') Power }.
```

Հարաբերության գործողությունները, ինչպես ամեն տեղ, վեցն են․ `=` \(հավասար է\), `<>` \(հավասար չէ\), `>` \(մեծ է\), `>=` \(մեծ է կամ հավասար\), `<` \(փոքր է\), `<=` \(փոքր է կամ հավասար\)։ `=` և `<>` գործողությունները կարելի է կիրառել նաև բուլյան արժեքների նկատմամբ։

```
Equality = Relation [ ('='|'<>') Relation ].
Relation = Addition [ ('>'|'>='|'<'|'<=') Addition ].
```

Արտահայտություններից ամենացածր նախապատվությունն ունեն _տրամաբանական_ գործողությունները՝ կանյունկցիան \(`And`\) և դիզյունկցիան \(`Or`\)։

```
   Disjunction = Conjunction { xOr Conjunction }.
   Conjunction = Equality { xAnd Equality }.
```

Ահա BASIC-IR լեզվի ամբողջ քերականությունը։ Այն հիմք է ծառայելու լեզվի կոմպիլյատորի իրականացման համար։
