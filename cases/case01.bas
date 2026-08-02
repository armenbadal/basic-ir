
SUB max(x, y)
  IF x + y > 0 THEN
    LET t = "Yes"
  END IF

  IF x > y THEN
    LET max = x
  ELSE
    LET max = y
  END IF

  LET sum = x + y
END SUB

SUB Main
  LET m1 = max(6, 1981)
  LET m2 = max(6, 1981)
END SUB
