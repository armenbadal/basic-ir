
'
' entry point
'
SUB Main
  ' declaration
  DIM x AS INTEGER
  ' assignment
  x = 777
  ' condition
  IF x > 1000 THEN
    PRINT TRUE
  ELSEIF x < 1000 THEN
    PRINT FALSE
  ELSE
    PRINT 0
  END IF
  ' loop 1
  WHILE x > 0
    PRINT x
    x = x - 1
  END WHILE
  '
  FOR x = 1 TO 10 STEP 2
    PRINT x
  END FOR
END SUB

