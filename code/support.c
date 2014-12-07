
/*
BASIC IR լեզվի ներդրված ֆունկցիաների գրադարան։
*/

extern int scanf(const char*, ...);
extern int printf(const char*, ...);
extern int putchar(int);

/* Ներմուծում */
int __input_integer__()
{
  int val = 0;
  scanf("%d", &val);
  return val;
}
double __input_double__()
{
  double val = 0.0;
  scanf("%lf", &val);
  return val;
}
_Bool __input_boolean__()
{
  return 0;
}

/* Արտածում */
void __print_integer__(int val)
{
  printf("%d", val);
}
void __print_double__(double val)
{
  printf("%lf", val);
}
void __print_boolean__(_Bool val)
{
  printf("%s", val ? "True" : "False");
}
void __print_space__()
{
  putchar(' ');
}
void __print_new_line__()
{
  putchar('\n');
}

/* Մաթեմատիկական ֆունկցիաներ ? 
 Օգտագործել LLVM֊ի ներդրված ֆունկցիաները։ */
double __f_pow__(double x, double y)
{
  return 0.0;
}
int __i_pow__(int x, int y)
{
  int val = 1;
  while( y != 0 ) {
    val *= x;
    --y;
  }
  return val;
}

/* Մուտքի կետ */
int main()
{
  extern void Main(void);
  Main();
  return 0;
}

