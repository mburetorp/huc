f (int x, int y)
{
  if (x % y != 0)
    abort();
}

main ()
{
  f (-5, 5);
  exit (0);
}