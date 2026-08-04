
SUB Main
  LET a = f(STR(7))
  CALL g STR(8), "Ok", a
END SUB

SUB f(e AS TEXT) AS TEXT
  LET f = MID(e, 2, 1)
END SUB

SUB g(e AS TEXT, k AS TEXT, m AS TEXT)
  LET a = e
END SUB

