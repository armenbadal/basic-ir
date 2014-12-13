
/*
BASIC IR լեզվի ներդրված ֆունկցիաների գրադարան։
*/

extern int scanf(const char*, ...);
extern int printf(const char*, ...);
extern int putchar(int);

/* Ներմուծում */
int __input_integer__()
{
  putchar('?'); putchar(' '); 
  int val = 0;
  scanf("%d", &val);
  return val;
}
double __input_double__()
{
  putchar('?'); putchar(' '); 
  double val = 0.0;
  scanf("%lf", &val);
  return val;
}
_Bool __input_boolean__()
{
  putchar('?'); putchar(' ');
  char val[8] = { 0 };
  scanf("%s", val);
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

/* Մուտքի կետ */
int main()
{
  extern void Main(void);
  Main();
  return 0;
}

