
#include "ast.hxx"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace llvm {
class BasicBlock;
class CallInst;
class Constant;
class Function;
class Module;
class Type;
class UnaryInstruction;
class Value;
} // namespace llvm

namespace basic {

///
class IrEmitter {
public:
    IrEmitter(llvm::LLVMContext& cx, llvm::Module& md);

    bool emitFor(ProgramPtr prog);

private:
    void emit(ProgramPtr prog);
    void emit(SubroutinePtr subr);

    void emit(StatementPtr st);
    void emit(SequencePtr seq);
    void emit(LetPtr let);
    void emit(InputPtr inp);
    void emit(PrintPtr pri);
    void emit(IfPtr sif);
    void emit(ForPtr sfor);
    void emit(WhilePtr swhi);
    void emit(CallPtr cal);

    llvm::Value* emit(ExpressionPtr expr);
    llvm::Value* emit(ApplyPtr apy);
    llvm::Value* emit(BinaryPtr bin);
    llvm::Value* emit(UnaryPtr una);
    llvm::Value* emit(TextPtr txt);
    llvm::Constant* emit(NumberPtr num);
    llvm::Constant* emit(BooleanPtr num);
    llvm::UnaryInstruction* emit(VariablePtr var);

    //! @brief BASIC-IR տիպից կառուցում է LLVM տիպ։
    llvm::Type* llvmType(Type type);
    llvm::Type* llvmType(std::string_view name);

    //! @brief Ճշտում է հերթական BasicBlock-ի դիրքը։
    void setCurrentBlock(llvm::Function* fun, llvm::BasicBlock* bl);

    void prepareLibrary();
    void declareLibraryFunction(std::string_view name, std::string_view signature);
    llvm::FunctionCallee libraryFunction(std::string_view name);
    llvm::FunctionCallee userFunction(std::string_view name);

    void createEntryPoint();
    void declareSubroutines(ProgramPtr prog);
    void defineSubroutines(ProgramPtr prog);
    bool createsTempText(ExpressionPtr expr);
    llvm::CallInst* createLibraryFuncCall(std::string_view fname,
            const llvm::ArrayRef<llvm::Value*>& args);

private:
    llvm::LLVMContext& context;
    llvm::IRBuilder<> builder;

    //! @brief Վերլուծված ծրագրի ծառը
    ProgramPtr prog;
    
    //! @brief Կառուցված մոդուլը
    llvm::Module& module;

    //! @brief Գրադարանային ֆունկցիաների ցուցակն է։
    //!
    //! Բանալին ֆունկցիայի անունն է, իսկ արժեքը ֆունկցիայի տիպն է՝
    //! որպես @c FunctionType ցուցիչ։ Այս ցուցակում պետք է գրանցվեն,
    //! բոլոր այն ֆունկցիաները, որոնք կոդի գեներատորն օգտագործելու է։
    std::unordered_map<std::string,llvm::FunctionType*> library;

    //! @brief Տեքստային հաստատունների ցուցակն է։
    //!
    //! Երբ պետք է գեներացվի հղում տեքստային հաստատունին, @c emitText
    //! մեթոդը նախ այդ հաստատունի հասցեն փնտրում է այս ցուցակում։ 
    //! Եթե տվյալ հաստատունն արդեն սահմանված է, ապա օգտագործվում
    //! է դրա հասցեն, հակառակ դեպքում՝ սահմանվում է նորը։
    std::unordered_map<std::string,llvm::Value*> globalTexts;

    //! @brief Ֆունկցիայի լոկալ անունների ցուցակն է։
    //!
    //! Սա օգտագործվում է բոլոր այն դեպքերում, երբ պետք է իդենտիֆիկատորը
    //! կապել @c Value ցուցիչի հետ։ Քանի որ վերլուծության վերջում արդեն 
    //! հայտնի են ենթածրագրում օգտագործված բոլոր անունները և դրանք 
    //! գրանցված են @c Subroutine օբյեկտի @c locals ցուցակում, ապա կոդի
    //! գեներացիայի ժամանակ հենց ամենասկզբում ստեղծվում է այս ցուցակը,
    //! իսկ այն դեպքերում, երբ պետք է հղվել անունին, համապատասխան
    //! ցուցիչն ընտրվում է այստեղից։
    std::unordered_map<std::string,llvm::Value*> varAddresses;

    //! @brief IR֊ը գեներացնելիս օգտագործվող տիպերը
    llvm::Type* VoidTy = builder.getVoidTy();
    llvm::Type* BooleanTy = builder.getInt1Ty();
    llvm::Type* NumericTy = builder.getDoubleTy();
    llvm::Type* TextualTy = builder.getInt8PtrTy();
};

} // namespace basic
