
SUB max(x AS REAL, y AS REAL) AS REAL
  DIM t AS REAL

  IF x + y > 0 THEN
    LET t = "Yes"
  END IF

  IF x > y THEN
    LET max = x
  ELSE
    LET max = y
  END IF

  DIM sum AS REAL
  LET sum = x + y
END SUB

SUB Main
  DIM m1 AS REAL
  LET m1 = max(6, 1981)
  DIM m2 AS REAL
  LET m2 = max(6, 1981)
END SUB
