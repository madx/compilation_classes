fact(entier $n)
{
  si $n == 1 alors
    renvoyer(1);
  sinon
    renvoyer ($n * fact($n - 1));
}

main()
entier $a;
{
  $a = lire ();
  ecrire (fact ($a));

  si $a < 3 alors renvoyer 1; sinon renvoyer 0;

}
.
