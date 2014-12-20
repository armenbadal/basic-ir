
extern int printf(const char*, ...);
extern void puts(const char*);
extern void hello();

int main()
{
  puts("Experiment");
  printf("Another experiment with %d number\n", 777);
  hello();
  return 0;
}

