extern void number_print(double);
extern void text_print(const char*);

double max(double x, double y)
{
    double max_r = 0.0;

  if( x + y > 0 )
    text_print("Yes");

  if( x > y )
    max_r = x;
  else
    max_r = y;

  number_print(x + y);

  return max_r;
}

void Main()
{
  number_print(max(6, 1981));
  number_print(max(6, 1981));
}

