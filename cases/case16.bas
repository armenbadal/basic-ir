
' երկու ներդրված WHILE
SUB f(x AS REAL, y AS REAL)
  WHILE x > y
    LET x = x - 1
    WHILE y < 10
      LET y = y + 1
      LET out = y
    END WHILE
  END WHILE
END SUB
