
#include "compiler.hxx"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>


class Arguments {
public:
    explicit Arguments(int argc, char* argv[])
    {
        for( int i = 0; i < argc; ++i )
            items.push_back(argv[i]);
    }

    auto count() const { return items.size(); }
    bool present(std::string_view arg) const
    {
        return items.end() != std::ranges::find(items, arg);
    }

    std::string item(int index) const
    {
        return items.at(index);
    }

private:
    std::vector<std::string> items;
};


int main(int argc, char* argv[])
{
    Arguments args(argc, argv);

    if( args.count() > 1 ) {
        basic::compile(
            args.item(1),
            args.present("--ir"),
            args.present("--lisp"));
    }

    return 0;
}
