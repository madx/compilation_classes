fact (entier $n)
{
  si $n <= 1 alors renvoyer 1; sinon renvoyer $n * fact($n - 1);
}

main()
{
  ecrire(fact(lire()));
}
.
