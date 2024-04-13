
#include "ast.hxx"
#include "astvisitor.hxx"

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
class IrEmitter : public ASTVisitor<void,void,void,llvm::Value*> {
public:
    IrEmitter(llvm::LLVMContext& cx, llvm::Module& md);

    bool emitFor(ProgramPtr prog);

private:
    // ծրագիր
    void visit(ProgramPtr prog) override;
    // ենթածրագիր
    void visit(SubroutinePtr subr) override;

    // ղեկավարող կառուցվածքներ
    void visit(StatementPtr st) override;
    void visit(SequencePtr seq) override;
    void visit(LetPtr let) override;
    void visit(InputPtr inp) override;
    void visit(PrintPtr pri) override;
    void visit(IfPtr sif) override;
    void visit(ForPtr sfor) override;
    void visit(WhilePtr swhi) override;
    void visit(CallPtr cal) override;

    // արտահայտություններ
    llvm::Value* visit(ExpressionPtr expr) override;
    llvm::Value* visit(ApplyPtr apy) override;
    llvm::Value* visit(BinaryPtr bin) override;
    llvm::Value* visit(UnaryPtr una) override;
    llvm::Value* visit(TextPtr txt) override;
    llvm::Value* visit(NumberPtr num) override;
    llvm::Value* visit(BooleanPtr num) override;
    llvm::Value* visit(VariablePtr var) override;

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

    //! @brief Կառուցված մոդուլը
    llvm::Module& moduler;

    llvm::IRBuilder<> builder;

    //! @brief Վերլուծված ծրագրի ծառը
    ProgramPtr prog;

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
    llvm::Type* TextualTy = builder.getPtrTy();
};

} // namespace basic
