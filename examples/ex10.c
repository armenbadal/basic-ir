
extern int scanf(const char*, ...);
extern int strcmp(const char*, const char*);

static const char* INTF = "%d";
static const char* DBLF = "%lf";

int inputInteger()
{
  int value = 0;
  scanf(INTF, &value);
  return value;
}

double inputDouble()
{
  double value = 0.0;
  scanf(DBLF, &value);
  return value;
}


_Bool inputBoolean()
{
  char value[8] = { 0 };
  scanf("%s", &value);
  if( 0 == strcmp(value, "True") )
    return 1;
  else if( 0 == strcmp(value, "False") )
    return 0;
  return 0;
}

void __printInteger__(int value)
{
  printf(INTF, value);
}

void __printDouble__(double value)
{
  printf(DBLF, value);
}

void __printBoolean__(_Bool value)
{
  printf("%s", value ? "True" : "False");
}

/**/
void u()
{
  int a = 7;
  __printInteger__(a);
  __printInteger__(8);

  double d = 3.2;
  __printDouble__(d);
  __printDouble__(4.8);

  _Bool b = 1;
  __printBoolean__(b);
  __printBoolean__(0);
}


