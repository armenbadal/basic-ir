
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <stdexcept>

class OptionLine {
private:
  static OptionLine* instance;
private:
  std::map<std::string,std::string> options;
  std::string input;
public:
  OptionLine* create()
  {
    if( instance == nullptr )
      instance = new OptionLine;
    return instance;
  }
  void declare(const std::string& name)
  {
    options[name] = "";
  }
  void parse(int argc, const char* argv[])
  {
    if( argc % 2 != 1 )
      throw new std::logic_error{"Error"};

    //for(int i = 0; i < argc; ++i )
  }
};

OptionLine::instance = nullptr;

/**/
int main(int argc, char* argv[])
{
  for( auto v : argv )
    std::cout << v << std::endl;
}

