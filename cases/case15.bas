
SUB s$
  LET s$ = "Ok"
END SUB

SUB f(x$, y)
  PRINT 3.14
  PRINT y
  
  PRINT "Halo!"
  PRINT x$
  PRINT "--> " & x$
  PRINT s$()
END SUB

