
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

