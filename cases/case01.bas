
SUB max(x, y, z)
  IF x > y THEN
    IF x > z THEN
        LET max = x
    ELSE 
        LET max = z
    END IF
  ELSE
    LET max = y
  END IF
  LET max = x + y
END SUB

