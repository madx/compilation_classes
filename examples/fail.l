main (entier $c)
entier $b, entier $i;
{
  $b = lire();
  $i = 0;

  tantque $i < 7 faire {
    $t[$i+1] = somme ($i, $b);
  }

  renvoyer 0;
}

main () {
  renvoyer 0;
}
.
