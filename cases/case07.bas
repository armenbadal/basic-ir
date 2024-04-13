
SUB IsPrime?(n)
  IF n <= 2 THEN
  	 LET IsPrime? = TRUE
  ELSEIF n MOD 2 = 0 THEN
    LET IsPrime? = FALSE
  ELSE
    LET k = 1 + SQR(n)
    WHILE (n MOD k <> 0) AND (k > 2)
      LET k = k - 1
    END WHILE
    LET IsPrime? = k <> 2
  END IF
END SUB

SUB Main
  LET e0? = IsPrime?(17)
  IF e0? THEN
    PRINT "Yes"
  END IF

  IF IsPrime?(16) THEN
    PRINT "Yes"
  ELSE
    PRINT "No"
  END IF
END SUB

