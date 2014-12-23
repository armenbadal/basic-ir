

#include <math.h>
#include <stdio.h>

int main()
{
  double pi = acos(-1);
  double v0 = sin(pi);
  double v1 = cos(pi);

  printf("sin(%lf) = %lf\n", pi, v0);
  printf("cos(%lf) = %lf\n", pi, v1);

  return 0;
}


