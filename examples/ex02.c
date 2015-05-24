
void Br1()
{
  int a = 0;
  int b = 0;
  if( a != b )
    a = 7;
}

void Br2()
{
  int a = 0;
  int b = 0;
  int c = 0;
  if( a != b )
    a = 7;
  else {
    b = 77;
    if( a > b ) 
      b = 5;
    else
      c = a * 2;
  }
}

void Main()
{
}


