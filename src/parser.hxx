
#include "ast.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace basic {
//! @brief Շարահյուսական վերլուծիչը
class Parser {
public:
    //! @brief Շարահյուսական վերլուծիչի կոնստրուկտորը
    //!
    //! @param filename - վերլուծվելիք ֆայլի ճանապարհը
    Parser(Scanner& sc);

    //! @brief Շարահյուսական վերլուծիչի դեստրուկտորը
    ~Parser();

    //! @brief Վերլուծում է ամբողջ ծրագիրը
    ProgramPtr parse();

private:
    //! @brief Վերլուծում է BASIC-IR ծրագիրը
    void parseProgram();

    //! @brief Վերլուծում է ենթածրագրի սահմանումը
    void parseSubroutine();

    //! @brief Վերլուծում է հրամանների հաջորդականությունը
    StatementPtr parseStatements();

    //! @brief Վերլուծում է մեկ հրաման
    StatementPtr parseOneStatement();

    //! @brief Վերլուծում է ներմուծման @c INPUT հրամանը
    StatementPtr parseInput();

    //! @brief Վերլուծում է արտածման @c PRINT հրամանը
    StatementPtr parsePrint();

    //! @brief Վերլուծում է վերագրման @c LET հրամանը
    StatementPtr parseLet();

    //! @brief Վերլուծում է ճյուղավորման @c IF հրամանը
    StatementPtr parseIf();

    //! @brief Վերլուծում է պայմանով ցիկլի @c WHILE հրամանը
    StatementPtr parseWhile();

    //! @brief Վերլուծում է պարամետրով ցիկլի @c FOR հրամանը
    StatementPtr parseFor();

    //! @brief Վերլուծում է պրոցեդուրայի կանչի @c CALL հրամանը
    StatementPtr parseCall();

    //! @brief Վերլուծում է համեմատման գործողությունները
    //!
    //! Վերլուծում է հավասար է, հավասար չէ, մեծ է, մեծ է կամ
    //! հավասար, փոքր է, փոքր է կամ հավասար գործողությունները։
    ExpressionPtr parseExpression();

    //! @brief Վերլուծում է ադիտիվ գործողությունները
    //!
    //! Վերլուծում է թվերի գումարման, հանման, տողերի կցման
    //! և դիզյունկցիայի գործողությունները։
    ExpressionPtr parseAddition();

    //! @brief Վերլուծում է մուլտիպլիկատիվ գործողությունները
    //!
    //! Վերլուծում է բազմապատկման, բաժանման մնացորդի որոշման
    //! և կոնյունկցիայի գործողությունները։
    ExpressionPtr parseMultiplication();

    //! @brief Վերլուծում է աստիճան բարձրացնելու գործողությունը
    ExpressionPtr parsePower();

    //! @brief Վերլուծում է պարզագույն արտահայտությունները
    //!
    //! Վերլուծում է թվային ու տեքստային հաստատունները, փոփոխականները,
    //! ֆունկցիայի կիրառումը և խմբավորման փակագծերի մեջ վերցրած
    //! արտահայտությունները։
    ExpressionPtr parseFactor();

    //! @brief Վերլուծում է նոր տողերի անցման նիշերը
    void parseNewLines();

    std::string match(Token tok);



private:
    Scanner& scanner;  //!< բառային վերլուծիչը
    Lexeme lookahead; //!< հերթական լեքսեմը

    //! @brief Վերլուծված ծրագրի ցուցիչը, միաժամանակ նաև
    //! վերլուծության ծառի արմատը
    ProgramPtr program;


};

} // basic
