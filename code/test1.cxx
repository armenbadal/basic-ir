
#include <iostream>

enum class Scope : bool {
  Local = true,
  Global = false
};

/**/
int main()
{
  Scope s{Scope::Local};
  std::cout << static_cast<bool>(s) << std::endl;
  s = Scope::Global;
  std::cout << static_cast<bool>(s) << std::endl;
}

