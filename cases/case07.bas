
SUB IsPrime(n)
  IF (n <= 2) OR (n MOD 2 = 0) THEN
    LET IsPrime = 1
  ELSE
    LET k = 1 + SQR(n)
    WHILE (n MOD k <> 0) AND (k > 2)
      LET k = k - 1
    END WHILE
    LET IsPrime = k <> 2
  END IF
END SUB
