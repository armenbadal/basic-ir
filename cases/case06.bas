
'
SUB Main
  CALL printOne
  CALL printTwo 3.1415
  CALL printThree 777, "Ok!"
END SUB

SUB printOne
  PRINT "Hi!"
END SUB

SUB printTwo(x)
  PRINT x
END SUB

SUB printThree(x, y$)
  PRINT x
  PRINT y$
END SUB
