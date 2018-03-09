
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
*/

/**/
char *text_input()
{
  char buffer[1024] = { 0 };
  fgets(buffer, 1024, stdin);
  char *res = malloc(1 + strlen(buffer));
  strcpy(res, buffer);
  return res;
}

/**/
char *text_clone(const char *sr)
{
  char *res = malloc(1 + strlen(sr));
  strcpy(res, sr);
  return res;
}

/**/
char *text_concatenate(const char *so, const char *si)
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
}

/**/
char *text_mid(const char *so, size_t b, size_t l)
{
  /* ենթատող */
}

/*
  համեմատման գործողություններ
*/

/**/
int text_equal(const char *so, const char *si)
{
  return 0 == strcmp(so, si);
}

/**/
int text_not_equal(const char *so, const char *si)
{
  return 0 != strcmp(so, si);
}

/**/
int text_greater(const char *so, const char *si)
{
  return 0 < strcmp(so, si);
}

/**/
int text_greater_or_equal(const char *so, const char *si)
{
  return 0 <= strcmp(so, si);
}

/**/
int text_lesser(const char *so, const char *si)
{
  return 0 > strcmp(so, si);
}

/**/
int text_lesser_or_equal(const char *so, const char *si)
{
  return 0 >= strcmp(so, si);
}

