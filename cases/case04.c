
extern double number_input(const char*);
extern void number_print(double);
extern void text_print(const char*);

void Main()
{
    double n = 0.0;
    n = number_input("?");
    while( n != 0 ) {
        number_print(n);
        n = n - 1;
    }
}

void f(const char *x, double y)
{
  text_print(x);
  number_print(y);
}
