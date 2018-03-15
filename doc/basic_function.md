
# Կոդի գեներացիա

## Ֆունկցիայի ստեղծումը

BASIC-IR լեզվի ենթածրագիրը թարգմանվում է LLVM համակարգի `Function` օբյեկտի։ Վերջինս LLVM IR լեզվի ենթածրագիրն է. ճիշտ այնպես, ինչպես `Subroutine`-ը BASIC-IR-ում։ `IrEmitter` դասում ենթածրագրերի թարմանությունն իրականացված է `emitFunction()` մեթոդում։

```C++
void IrEmitter::emitFunction( SubroutinePtr subr )
{
```

`Function` օբյեկտը ստեղծում ենք `Create` ստատիկ մեթոդով, որին պետք է տալստեղծվելիք ֆունկցիայի տիպը նկարագրող `FunctionType` օբյեկտի հասցե, ֆունկցիայի անունը և այն մոդուլի հասցեն, որում պետք է ավելացվի տվյալ ֆունկցիան։

```C++
static Function* llvm::Function::Create(
    FunctionType* Ty,
    LinkageTypes Linkage,
    const Twine& N = "",
    Module*	M = nullptr);
```

Ֆունկցիայի տիպը նկարագրող օբյեկտը կառուցում ենք ըստ `Subroutine` օբյեկտի պարամետրերի ու

```C++
static FunctionType* FunctionType::get(
    Type* Result,
    ArrayRef<Type*> Params,
    bool isVarArg);
```


Նախ կառուցվում է ապագա ֆունկցիայի տիպը։ Դրա համար պետք է ունենանք պարամետրերի տիպերի զանգվածը և վերադարձվող արժեքի տիպը։ Պարամետրերի տիպը որոշում ենք անվան կառուցվածքով։

```C++
    // պարամետրերի տիպերի ցուցակի կառուցումը
    std::vector<llvm::Type*> ptypes;
    for( auto& pr : subr->parameters )
        ptypes.push_back(llvmType(typeOf(pr)));
```

```C++
    // վերադարձվող արժեքի տիպը
    llvm::Type* rtype = nullptr;
    if( subr->hasValue )
        rtype = llvmType(typeOf(subr->name));
    else
        rtype = builder.getVoidTy();
```

```C++
    // ֆունկցիայի տիպը
    auto procty = llvm::FunctionType::get(rtype, ptypes, false);
    // ֆունկցիա օբյեկտը
    auto fun = llvm::Function::Create(procty,
        llvm::GlobalValue::ExternalLinkage,
        subr->name, module.get());
```




