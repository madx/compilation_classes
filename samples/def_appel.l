add(entier $x, entier $y)
entier $z;
{
$z = $x + $y;
renvoyer($z);
}

main() 
entier $a;
{
$a = add(lire(), lire());
ecrire($a);
}
.
