
SUB Main
  LET a$ = f$(STR$(7))
  CALL g STR$(8), "Ok", a$
END SUB

SUB f$(e$)
  LET f$ = MID$(e$, 2, 1)
END SUB

SUB g(e$, k$, m$)
  LET a$ = e$
END SUB

