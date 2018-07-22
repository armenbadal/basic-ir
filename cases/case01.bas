
SUB max(x, y)
  IF x + y > 0 THEN
    PRINT "Yes"
  END IF

  IF x > y THEN
    LET max = x
  ELSE
    LET max = y
  END IF

  PRINT x + y
END SUB

SUB Main
  PRINT max(6, 1981)
  PRINT max(6, 1981)
END SUB


