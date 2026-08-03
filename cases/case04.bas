
SUB Main
  DIM n AS REAL
  LET n = 0
  WHILE n <> 0
    DIM out AS REAL
    LET out = n
    LET n = n - 1
  END WHILE
END SUB

SUB f(x AS REAL, y AS REAL) AS REAL
  DIM a AS REAL
  LET a = x
  DIM b AS REAL
  LET b = y
END SUB
