#pragma once

#include "ast.hxx"
#include "diagnostics.hxx"
#include "lexeme.hxx"
#include "scanner.hxx"

#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace basic {

//! @brief Շարահյուսական վերլուծիչ
//!
//! Սխալների մշակումը կատարվում է Վիրտի նկարագրած պարզ եղանակով
//! («Compiler construction», 7.3)․
//!  - սխալի հանդիպելիս ոչինչ չի ընդհատվում. սխալը գրանցվում է @c mark()-ով,
//!    և վերլուծությունը շարունակվում է այնպես, կարծես բացակայող թոքենը կար,
//!  - հիմնական կառուցվածքների սկզբում դրված են համաժամեցման կետեր
//!    (@c sync()), որտեղ բաց են թողնվում այն թոքենները, որոնցով տվյալ
//!    կառուցվածքը սկսվել չի կարող։
//!
//! Այս եղանակը բացառություններ չի օգտագործում, ուստի վերլուծիչի ցանկացած
//! կետից կառավարումը շարունակվում է սովորական ձևով։
class Parser {
public:
    Parser(Scanner& sc);
    ~Parser();

    //! @brief Վերլուծում է ամբողջ ծրագիրը
    //!
    //! @warning Սխալների առկայության դեպքում վերադարձված ծառը թերի է։
    //! Կանչողը պարտավոր է նախ ստուգել @c hasErrors()-ը և միայն դրանից
    //! հետո ծառը փոխանցել հաջորդ փուլերին։
    Program::Ptr parse();

    //! @brief @c true, եթե գրանցվել է գոնե մեկ սխալ
    bool hasErrors() const noexcept;

    //! @brief Գրանցված սխալները՝ ի հայտ գալու հերթականությամբ
    const std::vector<Diagnostic>& getErrors() const noexcept;

private:
    Program::Ptr parseProgram();
    Subroutine::Ptr parseSubroutine();

    Statement::Ptr parseOneStatement();
    Sequence::Ptr parseSequence();
    Let::Ptr parseLet();
    Dim::Ptr parseDim();
    Input::Ptr parseInput();
    Print::Ptr parsePrint();
    If::IfThen::Ptr parseIfThen(bool first);
    If::Ptr parseIf();
    While::Ptr parseWhile();
    For::Ptr parseFor();
    Call::Ptr parseCall();

    std::vector<Expression::Ptr> parseExpressionList();
    Expression::Ptr parseExpression();
    Expression::Ptr parseAddition();
    Expression::Ptr parseMultiplication();
    Expression::Ptr parsePower();
    Expression::Ptr parseUnary();
    Expression::Ptr parseSubscript();
    Expression::Ptr parseFactor();
    Boolean::Ptr parseTrueOrFalse();
    Number::Ptr parseNumber();
    Text::Ptr parseText();
    Expression::Ptr parseArrayDefinition();
    Expression::Ptr parseIdentOrApply();
    Expression::Ptr parseGrouped();

    //! @brief Տողի ավարտը, ապա բոլոր հաջորդող դատարկ տողերը
    //!
    //! Ըստ քերականության՝ @c NewLines @c = @c EOL @c { @c EOL @c }., ուստի
    //! տողի ավարտի բացակայությունը սխալ է։ Այն տեղերում, որտեղ քերականությունը
    //! դատարկ տողերը թույլ է տալիս, բայց չի պարտադրում (@c [NewLines]),
    //! կանչը փակագծվում է @c lookahead.is(Token::NewLine) ստուգմամբ։
    void parseNewLines();

    //! @brief Բլոկի ավարտը՝ @c 'END' @c <keyword>
    void parseBlockEnd(Token keyword);

    void advance();

    //! @brief Ստուգում է ընթացիկ թոքենը և կլանում այն
    //!
    //! Անհամապատասխանության դեպքում գրանցում է սխալը, բայց վերլուծությունը
    //! չի ընդհատում. կատարվում է այնպես, կարծես բացակայող թոքենը տեղում էր։
    //!
    //! @return կլանված լեքսեմի արժեքը, կամ դատարկ տող՝ թոքենի բացակայության
    //! դեպքում։ Կանչերի մեծ մասն արժեքի կարիք չունի և անտեսում է այն։
    std::string match(Token tok);

    //! @brief Համաժամեցման կետ. բաց է թողնում ավելորդ թոքենները
    void sync(const std::set<Token>& stops, std::string_view message);
    void mark(std::string_view message);

private:
    Scanner& scanner;
    Lexeme lookahead;

    Diagnostics diagnostics;
    std::size_t tokenIndex = 0; //!< կլանված թոքենների քանակը՝ վերլուծիչի դիրքը
};
} // basic
