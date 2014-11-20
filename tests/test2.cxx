
#include <typeinfo>

/**/
class A {
public:
  virtual bool operator==(const A&) = 0;
};

class B : public A {
private:
  double val;
public:
  B(double v) : val{v} {}
  bool operator==(const A& o)
  {
    if( typeid(*this) != typeid(o) )
      return false;
    return val == dynamic_cast<const B&>(o).val;
  }
};


class C : public A {
private:
  char ch;
public:
  C(char c) : ch{c} {}
  bool operator==(const A& o)
  {
    if( typeid(*this) != typeid(o) )
      return false;
    return ch == dynamic_cast<const C&>(o).ch;
  }
};

#include <iostream>

/**/
int main()
{
  A* b0 = new B(3.14);
  A* b1 = new B(2.27);
  A* b2 = new B(2.27);
  std::cout << ((*b0)==(*b1)) << std::endl;
  std::cout << ((*b2)==(*b1)) << std::endl;

  std::cout << typeid(b0).name() << std::endl;
  std::cout << typeid(*b0).name() << std::endl;

  A* c0 = new C('A');
  A* c1 = new C('X');
  std::cout << ((*c0)==(*c1)) << std::endl;

  std::cout << typeid(c0).name() << std::endl;
  std::cout << typeid(*c0).name() << std::endl;

  std::cout << ((*b0)==(*c1)) << std::endl;
}

