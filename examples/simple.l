entier $b[1], entier $z;

add(entier $a, entier $b)
entier $sum;
{
  $sum = $a + $b;
  renvoyer $sum;
}

main()
entier $c[2], entier $a;
{
  $b[0] = 5;
  $a = lire();

  si $a != 4 alors {
    ecrire(2);
  } sinon {
    ecrire(4);
  }

  tantque $a < 2 faire {
    $a = $a + 1;
  }

  ecrire($a);

  renvoyer lire() + 3;
}
.
