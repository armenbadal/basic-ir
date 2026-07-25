#ifndef FORMATTERS_HXX
#define FORMATTERS_HXX

#include "ast.hxx"

#include <array>
#include <format>
#include <string_view>
#include <unordered_map>
#include <utility>

template<>
class std::formatter<basic::Operation> {
public:
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(basic::Operation op, std::format_context& ctx) const
    {
        using namespace basic;
        using namespace std::string_view_literals;
        static const std::unordered_map<basic::Operation,std::string_view> names{
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
        return std::format_to(ctx.out(), "{}", names.at(op));
    }
};

#endif // FORMATTERS_HXX
