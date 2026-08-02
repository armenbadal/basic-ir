
'
SUB Main
  CALL one
  CALL two 3.1415
  CALL three 777, "Ok!"
END SUB

SUB one
  LET a$ = "Hi!"
END SUB

SUB two(x)
  LET b = x
END SUB

SUB three(x, y$)
  LET c = x
  LET d$ = y$
END SUB
