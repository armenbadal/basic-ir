
#include "../code/parser.hxx"

/**/
int main()
{
  Parser po{"case1.bas"};
  auto ast = po.parse();
  delete ast;
}

