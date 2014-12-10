
# BASIC-IR լեզվի վերլուծությունը

Այս գլխում ես պատմում եմ BASIC-IR լեզվով գրված ծրագրերի տեքստից _աբստրակտ քերականական ծառի_ ստացման պրոցեսը։ Եվ քանի որ առաջնային նպատակը ծառի կառուցումն է, սկսեմ հենց դրա նկարագրությունից։


## Աբստրակտ քերականական ծառ

BASIC-IR լեզվով գրված ծրագրի տեքստի վերլուծության արդյունքում կառուցվող _աբստրակտ քերականական ծառը_ կարող է պարունակել երկու տիպի հանգույցներ․ `Expression` և `Statement`։ Այս երկուսն էլ աբստրակտ դասեր են, որոնցից առաջինից ժառանգվում են _արտահայտությունները_ մոդելավորող դասերը, իսկ երկրորդից՝ _հրամանները_ մոդելավորող դասերը։


### Կոդի գեներացիայի ինտերֆեյսները

`Expression` և `Statement` դասերն իրականացնում են նպատակային կոդ գեներացնող երկու ինտերֆեյսներ՝ LLVM IR կոդի գեներացիայի `CodeIR` ինտերֆեյսը, և աբստրակտ քերականական ծառի՝ _Lisp_ ներկայացման `LispAst` ինտերֆեյսը։

`CodeIR` ինտերֆեյսի `code` միակ վիրտուալ ֆունկցիան պատասխանատվություն է կրում իր արգումենտում տրված `IRBuilder` օբյեկտի օգնությամբ LLVM IR հրամանների հաջորդականության ստեղծման համար։

````c++
class CodeIR {
public:
  virtual llvm::Value* code(llvm::IRBuilder<>&) = 0;
};
````

`LispAst` ինտերֆեյսի `lisp` միակ վիրտուալ ֆունկցիան իր արգումենտում տրված `ostream` օբյեկտի մեջ է գրում աբստրակտ քերականական ծառի ամեն մի հանգույցի՝ Լիսպ լեզվի ստրուկտուրայի նմուշի տեսքով ներկայացումը։

````c++
class LispAst {
public:
  virtual void lisp(std::ostream&) = 0;
};
````

Աբստրակտ քերականական ծառի՝ Լիսպ լեզվի տվյալների տեսքով ներկայացումն ինձ օգնում է երկու գործում։ Նախ՝ շարահյուսական վերլուծությունից հետո միանգամից տեսնում եմ կառուցված ծառը։ Երկրորդ՝ հետագայում պատրաստվում եմ դրանց հիման վրա կոդի գեներատոր գրել ոչ միայլ LLVM IR լեզվի համար, այլ նաև ուրիշ միջանկյալ լեզուների համար (օրինակ, C--)։


### Կոմպիլյացիայի միավորի մոդելը

Նախաբանում նշեցի, որ BASIC-IR լեզվով գրված ծրագիրը՝ _կոմպիլյացիայի միավորը_, ֆունկցիաների ու պրոցեդուրաների սահմանումների (նաև հայտարարությունների) հաջորդականություն է։ `Module` դասը ես նախատեսել եմ կոմպիլյացիայի միովորը (մոդուլը) ներկայացնելու համար։ Այս դասի `name` դաշտը մոդուլի անունն է, այն ոչ մի իմաստ չի կրում և ես դրա մեջ գրում եմ կոմպիլյացիայի տրված ֆայլի անունը։ Իսկ `subs` դաշտը մոդուլում հայտարարված կամ սահմանված ենթածրագրերի ցուցակն է։

````c++
class Module : public CodeIR, public LispAst {
private:
  std::string name;
  std::vector<Function*> subs;
  /* ... */
public:
  Module(const std::string&);
  void addFunction(Function*);
  /* ... */
};
````


### Ենթածրագրերի մոդելը

BASIC-IR լեզվի _ֆունկցիաների_ ու _պրոցեդուրաների_ (_ենթածրագրերի_) մոդելները ներկայացնելու համար նախատեսել եմ `Function` դասը։ Սրա `name` դաշտը ցույց է տալիս ենթածրագրի անունը, `args` դաշտը արգումենտների ցուցակն է, որ աամեն մի տարրը անուն֊տիպ զույգ է, `type` դաշտը վերադարձվող արժեքի տիպն է, որը պրոցեդուրաների դեպքում ստանում է `VOID` պայմանական արժեքը և `body` դաշտը ենթածրագրի մարմինը կազմող հրամանների հաջորդականության ցուցիչն է։

````c++
class Function : public CodeIR, public LispAst {
public:
  std::string name;          // անուն
  vectornametype args;       // պարամետրեր
  std::string type;          // տիպ
  Statement* body = nullptr; // մարմին
  /* ... */
public:
  Function(const std::string&, const vectornametype&, const std::string&);
  /* ... */
};
````

Բացի `Expression` և `Statement` դասերից աբստրակտ երականական ծառի համար են նաև `Function` և `Module` դասերը։


### Արտահայտությունների մոդելը

Թվաբանական, համեմատման ու տրամաբանական արտահայտությունների համար բազային դաս է հանդիսանում `Expression` աբստրակտ դասը։ `env` ցուցիչը կապված է այն ենթածրագրի օբյեկտին, որի ներսում գտնվում է արտահայտությունը։ `type` դաշտը արտահայտության տիպը ցույց տվող ժառանգվող ատրիբուտ է։ `TyBoolean`, `TyInteger`, `TyDouble` և `TyVoid` տողային ստատիկ հաստատունները նախատեսված են BASIC-IR լեզվի ներդրված տիպերը նշանակելու համար։

````c++
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
````


#### Հաստատուններ

Երբ շարահյուսական անալիզատորը ծրագրում հանդիպում է ամբողջաթիվ, իրական կամ բուլյան հաստատուն, ապա ստեղծում է `Constant` տիպի օբյեկտ, որի `value` դաշտում պահում է հաստատունի արժեքը, իսկ `Expression` դասից ժառանգած `type` դաշտում՝ տիպը։

````c++
class Constant : public Expression {
private:
  std::string value;
public:
  Constant(const std::string& vl, const std::string& ty)
    : value{vl} { type = ty; }
  /* ... */
};
````


#### Փոփոխականներ

Փոփոխականների համար աբստրակտ քերականական ծառում ստեղծվում է `Variable` տիպի հանգույց։ Սրա `name` դաշտում պահվում է փոփոխականի անունը։

````c++
class Variable : public Expression {
private:
  std::string name;
public:
  Variable(const std::string& n, const std::string& t)
    : name{n} { type = t; }
  /* ... */
};
````

#### ՈՒնար գործողություններ

ՈՒնար գործողությունների համար նախատեսված `Unary` դասի `oper` դաշտում պահվում է գործողության անունը, օրինակ, `Not` կամ `Neg`, իսկ `expr` դաշտում՝ այն արտահայտությունն է, որի նկատմամբ պետք է կիրառել գործողությունը։

````c++
class Unary : public Expression {
private:
  std::string oper;
  Expression* expr;
public:
  Unary(const std::string&, Expression*);
  /* ... */
};
````

#### Բինար գործողություններ

Բինար գործողությունը մոդելավորող `Binary` դասը նույնպես պրունակում է `oper` դաշտը գործողությունների համար։ Ինչպես նաև `expro` և `expri` դաշտերը՝ գործողության երկու օպերանդներին ցույց տվող արտահայտությունների համար։

````c++
class Binary : public Expression {
private:
  std::string oper;
  Expression* expro;
  Expression* expri;
public:
  Binary(const std::string&, Expression*, Expression*);
  /* ... */
};
````

#### Ֆունկցիայի կանչ

Ֆունկցիայի կանչ կազմակերպող `FuncCall` դասը պարունակում է երկու դաշտ․ `name`, որը կանչվող ֆունկցիայի անունն է, և `args`, որը ֆունկցիային որպես արգումենտներ փոխանցվող արտահայտությունների ցուցակն է։

````c++
class FuncCall : public Expression {
private:
  std::string name;
  std::vector<Expression*> args;
public:
  FuncCall(const std::string& nm, const std::vector<Expression*>& ag)
    : name{nm}, args{ag} {}
  /* ... */
};
````


#### Տիպի փոփոխում

Երբ գործողություններ են կատարվում տարբեր `Integer` ու `Double` տիպերի միջև, BASIC-IR լեզվի շարահյուսական անալիզատորը, տիպերի համաձայնեցման համար, աբստրակտ քերականական ծառում ավելացնում է տիպի ձևափխման `TypeCast` հանգույցը։ Սա պարունակում է արտահայտության ցուցիչը և այն տիպի անունը, որին պետք է բերվի այդ արտահայտության արժեքը։

````c++
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
````



### Հրամանների մոդելը

Աբստրակտ քերականական ծառում BASIC-IR լեզվի հրամանները ներկայանում են `Statement` դասից ժառանգված դասերի տիպի հանգույցներով։ Ինչպես `Expression` դասը, այնպես էլ `Statement`֊ը պարունակում է `env` ցուցիչը։ Սա կապված է այն `Function` օբյեկտին, որի մարմնում հանդիպել է տվյալ հրամանը։

````c++
class Statement : public CodeIR, public LispAst {
protected:
  Function* env;
public:
  virtual ~Statement() {}
  virtual void setEnv(Function* e) { env = e; }
};
````


#### Փոփոխականի հայտարարում



````c++
class Declare : public Statement {
private:
  std::string name;
  std::string type;
public:
  Declare(const std::string& n, const std::string& t)
    : name{n}, type{t} {}
  /* ... */
};
````

#### Վերագրում

#### Պրոցեդուրայի կանչ

#### Արդյունքի վերադարձ ֆունկցիայից

#### Ճյուղավորում

#### Նախապայմանով ցիկլ

#### Հաշվիչով ցիկլ

#### Ներմուծում և արտածում



## Նիշային վերլուծություն

Նիշային վերլուծության ընթացքում ծրագրի տեքստից պետք է առանձնացնել հետևյալ տիպի _լեքսեմները_ և նրանց նամապատասխանեցնել որոշակի _թոքեններ_․

* _Ծառայողական բառեր_, ինչպիսիք են, օրինակ, `If`, 'Dim`, `End`, `And` և այլն։ 
* _Իդենտիֆիկատորներ_, որոնք սկսվում են տառով և կարող են պարունակեմ տառեր ու թվանշաններ։ Օրինակ, `w`, `el`, `x0`, `point2a`։
* _Իրական_ և _ամբողջ_ թվեր։ Օրինակ, `12`, `777`, `3.14`, `0.84`։
* _Գործողություններ_ ցույց տվող նիշեր կամ դրանց միացություններ։ Օրինակ, ՝,՝, `=`, `<>`, `+`, `*`։

Բացի նշված լեքսեմները ճանաչելուց, նիշային վերլուծիչը պետք է դեն նետի ծրագրի տեքստում հանդիպող _մեկնաբանությունները_, որոնք սկսվում են `'` (ապաթարց) նիշով և շարունակվում են միչև տողի վերջը։


### Թոքեններ

Նիշային վերլուծության և շարահյուսական վերլուծության միջև երկխոսությունը կազմակերպվում է `Token` թվարկմամբ սահմանված անունների «լեզվով»։ 

````c++
enum Token {
  xNull,        // անհայտ
  xEol,         // տողի վերջ
  xInteger,     // ամբողջ թիվ
  xDouble,      // իրական թիվ
  xTrue,        // True   բուլյան ԿԵՂԾ հաստատունը
  xFalse,       // False  բուլյան ՃՇՄԱՐԻՏ հաստատունը
  xIdent,       // իդենտիֆիկատոր
  xDim,         // Dim
  xAs,          // As
  xType,        // Type
  xEnd,         // End
  xDeclare,     // Declare
  xSubroutine,  // Sub
  xFunction,    // Function
  xReturn,      // Return
  xIf,          // If
  xThen,        // Then
  xElseIf,      // ElseIf
  xElse,        // Else
  xFor,         // For
  xTo,          // To
  xStep,        // Step
  xWhile,       // While
  xInput,       // Input
  xPrint,       // Print
  xLPar,        // (   ձախ փակագիծ
  xRPar,        // )   աջ փակագիծ
  xComma,       // ,   ստորակետ
  xAnd,         // And կոնյունկցիա
  xOr,          // Or  դիզյունկցիա
  xNot,         // Not ժխտում
  xEq,          // =   հավասար է
  xNe,          // <>  հավասար չէ
  xGt,          // >   մեծ է
  xGe,          // >=  մեծ է կամ հավասար
  xLt,          // <   փոքր է
  xLe,          // <=  փոքր է կամ հավասար
  xAdd,         // +   գումարում
  xSub,         // -   հանում
  xMul,         // *   բազմապատկում
  xDiv,         // /   բաժանման քանորդի որոշում
  xMod,         // \   բաժանման մնացորդի որոշում
  xPow,         // ^   աստիճան բարձրացնել
  xEof          // ֆայլի վերջ
};
````


### Լեքսեմների ճանաչումը

#### `Scanner` դասը

````c++
class Scanner {
private:
  std::ifstream source;
  char c;

  int linenum{1};
  std::string text{""};
  static std::map<std::string,Token> keywords;

public:
  Scanner(const std::string&);
  std::string lexeme() const;
  inline int line() const { return linenum; }
  Token next();

private:
  std::string sequence(std::function<bool(char)>);
};
````




