# Լոկալների օրինակ

## BASIC-IR

SUB max(a[] AS REAL) AS REAL
  LET max = a[0]
  FOR i = 1 TO LEN(a)
    IF a[i] > max THEN
      LET max = a[i]
    END IF
  END FOR
END SUB


---------------------------
Parameters:
a[] AS REAL 

Locals։
i AS REAL


## C

double max(double a[], size_t n)
{
    double m = a[0];
    for( int i = 1; i < n; ++i )
        if( a[i] > m )
            m = a[i];
    return m;
}

struct _array_descriptor {
    void *elements;
    size_t size;
};
