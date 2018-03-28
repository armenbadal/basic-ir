
' երկու ներդրված WHILE
SUB f(x, y)
  WHILE x > y
    LET x = x - 1
    WHILE y < 10
      LET y = y + 1
      PRINT y
    END WHILE
    'PRINT x
  END WHILE
END SUB

