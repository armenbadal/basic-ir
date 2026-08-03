
'
'
'
SUB Main
  FOR i = 0 TO 10 STEP 2
    DIM sq AS REAL
    LET sq = i * i
  END FOR

  FOR j = 0 TO 10
    DIM cube AS REAL
    LET cube = j * j * j
  END FOR

  FOR k = 0 TO 10 STEP -4
    DIM mul AS REAL
    LET mul = 77 * k
  END FOR
END SUB
