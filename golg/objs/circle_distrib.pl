#!/usr/bin/perl

$n = 500;

for ($i=0; $i<$n; $i++)
{
    $x[$i] = rand(10) - 5.0;
    $y[$i] = rand(10) - 5.0;
}

for ($i=0; $i<$n; $i++)
{
    for ($j=$i+1; $j<$n; $j++)
    {
        $a = $x[$i]*$x[$i] + $y[$i]*$y[$i];
        $b = $x[$j]*$x[$j] + $y[$j]*$y[$j];
        if ($a>$b)
        {
            $t = $x[$i];
            $x[$i] = $x[$j];
            $x[$j] = $t;

            $t = $y[$i];
            $y[$i] = $y[$j];
            $y[$j] = $t;
        }
    }
}

print
    "static i4_float distrib[][3] =\n" .
    "{\n";
for ($i=0; $i<50; $i++)
{
    $a = $x[$i]*$x[$i] + $y[$i]*$y[$i];
    printf("  { %10.7f, %10.7f, %10.7f },\n", $x[$i] , $y[$i], $a);
}
print "};\n";
