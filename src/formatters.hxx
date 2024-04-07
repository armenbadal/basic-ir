#ifndef FORMATTERS_HXX
#define FORMATTERS_HXX

#include "ast.hxx"

#include <array>
#include <format>
#include <string_view>
#include <unordered_map>
#include <utility>

template<>
class std::formatter<basic::Operation> : public std::formatter<std::string_view> {
public:
    auto format(basic::Operation op, std::format_context& ctx) const
    {
        using namespace basic;
        using namespace std::string_view_literals;
        static std::unordered_map<basic::Operation,std::string_view> names{
            { Operation::None, "None"sv },
            { Operation::Add,   "+"sv },
            { Operation::Sub,   "-"sv },
            { Operation::Mul,   "*"sv },
            { Operation::Div,   "/"sv },
            { Operation::Mod,   "\\"sv },
            { Operation::Pow,   "^"sv },
            { Operation::Eq,   "="sv },
            { Operation::Ne,   "<>"sv },
            { Operation::Gt,   ">"sv },
            { Operation::Ge,   ">="sv },
            { Operation::Lt,   "<"sv },
            { Operation::Le,   "<="sv },
            { Operation::And,  "AND"sv },
            { Operation::Or,   "OR"sv },
            { Operation::Not,  "NOT"sv },
            { Operation::Conc, "&"sv }
        };
        return std::formatter<std::string_view>::format(names[op], ctx);
    }
};

template<>
class std::formatter<basic::Type> : public std::formatter<std::string_view> {
public:
    auto format(basic::Type tp, std::format_context& ctx) const
    {
        using namespace basic;
        using namespace std::string_view_literals;
        static std::unordered_map<basic::Type,std::string_view> names{
            { Type::Void,    "VOID"sv },
            { Type::Boolean, "BOOLEAN"sv },
            { Type::Numeric, "NUMBER"sv },
            { Type::Textual, "TEXT"sv }
        };
        return std::formatter<std::string_view>::format(names[tp], ctx);
    }
};

#endif // FORMATTERS_HXX
