
int __i_pow__(int x, int y)
{
  if( y == 0 ) return 1;
  if( y == 1 ) return x;
  return x * __i_pow__(x, y - 1);
}

