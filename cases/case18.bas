
SUB Gcd(x AS REAL, y AS REAL) AS REAL
    WHILE x * y <> 0
        IF x > y THEN
            LET x = x - y
        ELSE
            LET y = y - x
        END IF
    END WHILE
    LET Gcd = x + y
END SUB

SUB Main
    LET result = Gcd(18, 192)
END SUB
