
'
SUB Main
  CALL one
  CALL two 3.1415
  CALL three 777, "Ok!"
END SUB

SUB one
  LET a = "Hi!"
END SUB

SUB two(x AS REAL)
  LET b = x
END SUB

SUB three(x AS REAL, y AS TEXT)
  LET c = x
  LET d = y
END SUB
