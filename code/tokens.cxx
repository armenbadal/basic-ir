
#include <map>
#include <string>
#include <vector>

#include "tokens.hxx"


namespace basic {
  /* Թոքենների անունները, հաղորդագրություններում օգտագործելու համար */
  std::vector<std::string> names = {
    "NIL", "EOL", "Integer", "Double", "True", "False", 
    "Ident", "Dim", "As", "Type", "End", "Declare", "Sub", 
    "Function", "Return", "If", "Then", "ElseIf", "Else",
    "For", "To", "Step", "While", "Input", "Print",
    "(", ")", ",", "And", "Or", "Not", "=", "<>", ">", 
    ">=", "<", "<=", "+", "-", "*", "/", "\\", "^", "EOF"
  };

  std::string N( Token k )
  {
    return names[static_cast<size_t>(k)];
  }
}



