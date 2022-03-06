
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/*
  թվային ֆունկցիաներ
*/

/**/
double number_input(const char* prompt)
{
  printf("%s ", prompt);
	double nval = 0.0;
	scanf("%lf", &nval);
	while( '\n' != getchar() ) {}
  return nval;
}

/**/
void number_print(double vl)
{
    printf("%lf\n", vl);
}


/*
  տեքստային ֆունկցիաներ
*/

/**/
char *text_input(const char* prompt)
{
  printf("%s ", prompt);
  char buffer[1024] = { 0 };
  fgets(buffer, 1023, stdin);
  size_t slen = strlen(buffer);
  char *res = malloc(slen);
  strncpy(res, buffer, slen - 1);
  res[slen-1] = '\0';
  return res;
}

/**/
void text_print(const char* vl)
{
    printf("%s\n", vl);
}

/**/
char *text_clone(const char *sr)
{
  char *res = malloc(1 + strlen(sr));
  strcpy(res, sr);
  return res;
}

/**/
char *text_conc(const char *so, const char *si)
{
  size_t len = 1 + strlen(so) + strlen(si);
  char *res = malloc(len);
  strcpy(res, so);
  strcat(res, si);
  return res;
}

/**/
char *text_str(double nv)
{
  /* թվի տեքտային տեսքը */
  return NULL;
}

/**/
char *text_mid(const char *so, double b, double l)
{
  /* ենթատող */
  return NULL;
}

/*
  համեմատման գործողություններ
*/

/**/
bool text_eq(const char *so, const char *si)
{
  return 0 == strcmp(so, si);
}

/**/
bool text_ne(const char *so, const char *si)
{
  return 0 != strcmp(so, si);
}

/**/
bool text_gt(const char *so, const char *si)
{
  return 0 < strcmp(so, si);
}

/**/
bool text_ge(const char *so, const char *si)
{
  return 0 <= strcmp(so, si);
}

/**/
bool text_lt(const char *so, const char *si)
{
  return 0 > strcmp(so, si);
}

/**/
bool text_le(const char *so, const char *si)
{
  return 0 >= strcmp(so, si);
}

