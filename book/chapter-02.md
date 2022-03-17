# Վերլուծության ծառ

BASIC-IR լեզվով գրված ծրագրի թարգմանությունը բաղկացած է երկու մեծ քայլերից․ _վերլուծություն_ \(analysis\) և _համադրում_ \(synthesis\)։ Վերլուծության փուլում ծրագրի տեքստը ճանաչվում է շարահյուսական անալիզատորի կողմից և լեզվական ամեն մի բաղադրիչի համար ստեղծվում է համապատասխան տվյալների կառուցվածքի նմուշը։ Համադրման փուլում այդ կառուցվածքներից ստեղծվում է նպատակային կոդը \(տվյալ դեպքում դա LLVM IR կոդն է\)։

Վելուծության արդյունքում ստեղծված տվյալների կառուցվածքը կոչվում է _աբստրակտ շարահյուսական ծառ_ կամ _վերլուծության ծառ_։ Օրինակ, ունենք BASIC-IR լեզվով գրված ծրագրի հետևյալ հատվածը.

```basic
WHILE n * m <> 0
  IF n > m THEN
    LET n = n MOD m
  ELSE
    LET m = m MOD n
  END IF
END WHILE
```

Ծառի հանգույցի ամեն մի տեսակի կառուցվածքը անմիջապես բխում է տվյալ հրամանի կամ արտահայտության քերականական կանոնից։ Օրինակ, պայմանով ցիկլի քերականությունը այսպիսինն էր․

```
While = 'WHILE' Expression Statements 'END' 'WHILE'.
```

Այս քերականական հավասարումից երևում է, որ `WHILE` կառուցվածքը ներկայացնող աբստրակտ հանգույցը պետք է ունենա երկու բաղադրիչ՝ հղումներ ցիկլի պայմանը ներկայացնող արտահայտության և մարմինը ներկայացնող հրամանների ենթածառին։

Իսկ վերը բերված ծրագրի հատվածին համապատասխանող աբստրակտ շարահյուսական ծառը կարող է ունենալ հետևյալ տեսքը․

```
|          WHILE
|    <>                IF
|  *    0      >       =       =
| n m         n m    n   \   m   \
|                       n m     m n
```

BASIC-IR լեզվով գրված ծրագրի տեքստի վերլուծության արդյունքում կառուցվող աբստրակտ շարահյուսական ծառը կարող է պարունակել չորս տիպի հանգույցներ՝ մոդելավորված հետևյալ դասերով․

* `Program` ― _կոմպիլյացիայի միավոր_։ Քանի որ կոմպիլյատորը կարող է թարգմանել միայն մեկ կոմպիլյացիայի միավոր, ստեղծվում է `Program` դասի միակ նմուշ։
* `Subrotine` ― _ենթածրագիր_։ Կոմպիլյացիայի միավորում հանդիպող յուրաքանչյուր ենթածրագրի համար ստեղծվում է `Subroutine` դասի մեկ նմուշ։
* `Statement` ― _հրաման_։ Սա աբստրակտ դաս է, որից ժառանգված են կոնկրետ հրամանների դասերը՝ `Sequence` \(հաջորդականություն\), `Call` \(պրոցեդուրայի կանչ\), `Let` \(վերագրում\), `If` \(ճյուղավորում\), `For` \(հաշվիչով ցիկլ\), `While` \(պայմանով ցիկլ\), `Input` \(ներմուծում\) և `Print` \(արտածում\)։
* `Expression` ― _արտահայտություն_։ Նույնպես աբստրակտ դաս է, որից ժառանգված են `Variable` \(փոփոխական\), `Number` \(թիվ\), `Text` \(տեքստ\), `Unary` \(ունար գործողություն\)`Binary` \(բինար գործողություն\) և `Apply` \(ֆունկցիայի կիրառում\)։

Բոլոր այս նշված դասերը ժառանգած են `AstNode` վերացական դասից։

```cpp
class AstNode {
public:
    AstNode();
    virtual ~AstNode() = default;

    static void deleteAllocatedNodes();

public:
    NodeKind kind = NodeKind::Empty;
    unsigned int line = 0;

private:
    static list<AstNode*> allocatedNodes;
};
```

## Ծրագիր մոդելը

Վերլուծության ծառում BASIC-IR ծրագրի ներկայացման համար նախատեսված է `Program` դասը։

```cpp
class Program : public AstNode {
public:
    string filename = "";
    vector<Subroutine*> members;

public:
    Program(const string& fn);
};
```

## Ենթածրագրի մոդելը

BASIC-IR լեզվի _ֆունկցիաների_ ու _պրոցեդուրաների_ \(_ենթածրագրերի_\) մոդելները ներկայացնելու համար նախատեսել եմ `Function` դասը։ Սրա `name` դաշտը ցույց է տալիս ենթածրագրի անունը, `args` դաշտը արգումենտների ցուցակն է, որի ամեն մի տարրը անուն֊տիպ զույգ է, `type` դաշտը վերադարձվող արժեքի տիպն է, որը պրոցեդուրաների դեպքում ստանում է `VOID` պայմանական արժեքը և `body` դաշտը ենթածրագրի մարմինը կազմող հրամանների հաջորդականության ցուցիչն է։

```cpp
class Subroutine : public AstNode {
public:
    string name = "";
    vector<string> parameters;
    bool isBuiltIn = false;
    vector<Variable*> locals;
    Statement* body = nullptr;
    bool hasValue = false;

public:
    Subroutine(const string& nm, const vector<string>& ps);
};
```

## Հրամանների մոդելը

Աբստրակտ շարահյուսական ծառում BASIC-IR լեզվի հրամանները ներկայանում են `Statement` աբստրակտ դասից ժառանգված դասերի տիպի հանգույցներով։ `env` \(environment ― միջավայր\) ցուցիչը կապված է այն ենթածրագրի օբյեկտին, որի ներսում գտնվում է հրամանը։

```cpp
class Statement : public AstNode {};
```

`Statement` դասից ժառանգված ստորև թվարկվող դասերի համար ես բաց կթողնեմ `code()`, `lisp()` և `setEnv()` մեդոդների հայտարարությունները, եթե իհարկե որևէ առանձնահատուկ դեպք չէ։

### Հրամանների հաջորդում

BASIC-IR-ը պրոցեդուրային լեզու է, և նրանում հատկապես կարևոր է, որ հրամանները կատարվեն ճիշտ այն հաջորդականությամբ, ինչպես գրված է ծրագրի տեքստում։ `Sequence` դասը նախատեսված է այդ կարգը պահպանելու համար։

```c++
class Sequence : public Statement {
public:
    vector<Statement*> items;

public:
    Sequence();
};
```

### Փոփոխականի հայտարարում

Փոփոխականի հայտարարման \(տվյալ դեպքում՝ նաև սահմանման\) համար անհրաժեշտ են փոփոխականի անունն ու դրա տիպը որոշող իդենտիֆիկատորը։ `Declare` դասի `name` և `type` անդամները

```c++
class Declare : public Statement {
private:
  std::string name; // անուն
  std::string type; // տիպ

public:
  Declare(const std::string& n, const std::string& t)
    : name{n}, type{t} {}
  /* ... */
};
```

### Վերագրում

Վերագրման հրամանը տրված անունով փոփոխականի վերագրում է տրված արտահայտության արժեքը։ `Assign` դասի `name` դաշտը փոփոխականի անունն է, իսկ `expr` անդամը՝ արտահայտության ցուցիչն է։

```c++
class Assign : public Statement {
private:
  std::string name; // փոփոխականի անուն
  Expression* expr; // վերագրվող արտահայտություն

public:
  Assign(const std::string& n, Expression* e)
    : name{n}, expr{e} {}
  /* ... */
};
```

### Պրոցեդուրայի կանչ

Պրոցեդուրայի կանչի հրամանը նույնն է, ինչ ֆունկցիայի կանչը․․․։

### Արդյունքի վերադարձ ֆունկցիայից

`Return` հրամանը ընդհատում է ֆունկցիայի աշխատանքը և ֆունկցիայի կանչ կետին է վերադարձնում իր արգումենտում տրված արտահայտության արժեքը։ `Result` դասի `exp` անդամը հենց այդ արտահայտության ցուցիչն է։

```c++
class Result : public Statement {
private:
  Expression* exp;

public:
  Result(Expression* e) : exp{e} {}
  /* ... */
};
```

### Ճյուղավորում

```c++
class Branch : public Statement {
private:
  Expression* cond;
  Statement* thenp;
  Statement* elsep;

public:
  Branch(Expression* c, Statement* t, Statement* e)
    : cond{c}, thenp{t}, elsep{e} {}
  void setElse(Statement* s) { elsep = s; }
  /* ... */
};
```

### Նախապայմանով ցիկլ

```c++
class WhileLoop : public Statement {
private:
  Expression* cond;
  Statement* body;

public:
  WhileLoop(Expression* co, Statement* bo)
    : cond{co}, body{bo} {}
  /* ... */
};
```

### Հաշվիչով ցիկլ

```c++
class ForLoop : public Statement {
private:
  std::string param;
  Expression* start;
  Expression* stop;
  Expression* step;
  Statement* body;

public:
  ForLoop(const std::string& pr, Expression* sa, Expression* so, Expression* se, Statement* bo)
    : param{pr}, start{sa}, stop{so}, step{se}, body{bo} {}
  /* ... */
};
```

### Ներմուծում և արտածում

```c++
class Input : public Statement {
private:
  symbolvector vars;
public:
  Input(const symbolvector& vs)
    : vars{vs} {}
  /* ... */
};
```

```c++
class Print : public Statement {
private:
  std::vector<Expression*> vals;
public:
  Print(const std::vector<Expression*>& vl)
    : vals{vl} {}
  /* ... */
};
```

## Արտահայտությունների մոդելը

Թվաբանական, համեմատման ու տրամաբանական արտահայտությունների համար բազային դաս է հանդիսանում `Expression` աբստրակտ դասը։ Ինչպես `Statement` դասը, այնպես էլ `Expression`֊ը պարունակում է `env` ցուցիչը։ Սա կապված է այն `Function` օբյեկտին, որի մարմնում հանդիպել է տվյալ հրամանը։ `type` դաշտը արտահայտության տիպը ցույց տվող ժառանգվող ատրիբուտ է։ `TyBoolean`, `TyInteger`, `TyDouble` և `TyVoid` տողային ստատիկ հաստատունները նախատեսված են BASIC-IR լեզվի ներդրված տիպերը նշանակելու համար։

```c++
class Expression : public CodeIR, public LispAst {
public:
  static const std::string TyBoolean;
  static const std::string TyInteger;
  static const std::string TyDouble;
  static const std::string TyVoid;
protected:
  Function* env{nullptr};
public:
  std::string type;
public:
  virtual ~Expression() {}
  virtual void setEnv(Function* e) { env = e; }
};
```

### Հաստատուններ

Երբ շարահյուսական անալիզատորը ծրագրում հանդիպում է ամբողջաթիվ, իրական կամ բուլյան հաստատուն, ապա ստեղծում է `Constant` տիպի օբյեկտ, որի `value` դաշտում պահում է հաստատունի արժեքը, իսկ `Expression` դասից ժառանգած `type` դաշտում՝ տիպը։

```c++
class Constant : public Expression {
private:
  std::string value;
public:
  Constant(const std::string& vl, const std::string& ty)
    : value{vl} { type = ty; }
  /* ... */
};
```

### Փոփոխականներ

Փոփոխականների համար աբստրակտ շարահյուսական ծառում ստեղծվում է `Variable` տիպի հանգույց։ Սրա `name` դաշտում պահվում է փոփոխականի անունը։

```c++
class Variable : public Expression {
private:
  std::string name;
public:
  Variable(const std::string& n, const std::string& t)
    : name{n} { type = t; }
  /* ... */
};
```

### ՈՒնար գործողություններ

ՈՒնար գործողությունների համար նախատեսված `Unary` դասի `oper` դաշտում պահվում է գործողության անունը, օրինակ, `Not` կամ `Neg`, իսկ `expr` դաշտում՝ այն արտահայտությունն է, որի նկատմամբ պետք է կիրառել գործողությունը։

```c++
class Unary : public Expression {
private:
  std::string oper;
  Expression* expr;
public:
  Unary(const std::string&, Expression*);
  /* ... */
};
```

### Բինար գործողություններ

Բինար գործողությունը մոդելավորող `Binary` դասը նույնպես պրունակում է `oper` դաշտը գործողությունների համար։ Ինչպես նաև `expro` և `expri` դաշտերը՝ գործողության երկու օպերանդներին ցույց տվող արտահայտությունների համար։

```c++
class Binary : public Expression {
private:
  std::string oper;
  Expression* expro;
  Expression* expri;
public:
  Binary(const std::string&, Expression*, Expression*);
  /* ... */
};
```

### Ֆունկցիայի կանչ

Ֆունկցիայի կանչ կազմակերպող `FuncCall` դասը պարունակում է երկու դաշտ․ `name`, որը կանչվող ֆունկցիայի անունն է, և `args`, որը ֆունկցիային որպես արգումենտներ փոխանցվող արտահայտությունների ցուցակն է։

```c++
class FuncCall : public Expression {
private:
  std::string name;
  std::vector<Expression*> args;
public:
  FuncCall(const std::string& nm, const std::vector<Expression*>& ag)
    : name{nm}, args{ag} {}
  /* ... */
};
```

### Տիպի փոփոխում

Երբ գործողություններ են կատարվում տարբեր `Integer` ու `Double` տիպերի միջև, BASIC-IR լեզվի շարահյուսական անալիզատորը, տիպերի համաձայնեցման համար, աբստրակտ շարահյուսական ծառում ավելացնում է տիպի ձևափոխման `TypeCast` հանգույցը։ Սա պարունակում է արտահայտության ցուցիչը և այն տիպի անունը, որին պետք է բերվի այդ արտահայտության արժեքը։

```c++
class TypeCast : public Expression {
private:
  Expression* expr;
  std::string from; // ?? հեռացնել
  std::string to;
public:
  TypeCast(Expression* e, const std::string& f, const std::string& t)
    : expr{e}, from{f}, to{t} { type = to; }
  /* ... */
};
```



