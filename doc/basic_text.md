# Տեքստերի ներկայացման մասին

Տեքստը չփոփոխվող (immutable) օբյեկտ է։ Վերագրման գործողության 
ժամանակ այն պատճենվում է, իս արդեն ոչ պետքական նմուշները 
ոչնչացվում են։ Տեքստային օբյեկտ վերադարձնող ֆունկցիաները միշտ
վերադարձնում են նոր ստեղծված օբյեկտ։

Օրինակ, `&` գործողության համար կարելի է գրել այսպիսի մի ֆունկցիա.

```c
char *text_concatenate(const char *so, const char *si)
{
  char *res = malloc(1 + strlen(so) + strlen(si));
  strcpy(res, so);
  strcat(res, si);
  return res;
}
```

Իսկ երբ BASIC-IR ծրագրում հանդիպում է `LET a$ = "Ok" & "?"` տեսքի
հրաման, ապա պետք է կազմակերպել մոտավորապես հետևյալ տիպի կոդ։

```c
const char *g_str_0 = "Ok";
const char *g_str_1 = "?";

/* ... */

char *temp_0 = text_concatenate(g_str_0, g_str_1);
free(a);
char *a = text_clone(temp_0);
free(temp_0);

/* ... */
```
