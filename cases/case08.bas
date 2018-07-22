
SUB f(x)
  LET f = x
  LET f = -x
  LET f = NOT x
END SUB

SUB g(x ,y)
  LET g = x + y
  LET g = x - y
  LET g = x * y
  LET g = x / y
  LET g = x MOD y
  LET g = x AND y
  LET g = x OR y
  LET g = x = y
  LET g = x <> y
  LET g = x > y
  LET g = x >= y
  LET g = x < y
  LET g = x <= y
END SUB

SUB h(x, y)
  LET h = (x + y) OR (x - y)
  LET h = (x + y) AND (x - y)

  LET h = (x = y) OR (x <> y)
  LET h = (x = y) AND (x <> y)

  LET h = (x > y) OR (x < y)
  LET h = (x > y) AND (x < y)
END SUB

