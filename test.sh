#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./toycc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;" 
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 "12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 4 "(3+5+1)/2;"
assert 10 "-10+20;"
assert 200 "-10*-20;"
assert 2 "-20/-10;"
assert 5 "+20/+4;"
assert 1 "20>5;"
assert 0 "2>5;"
assert 1 "20>=20;"
assert 0 "20>=21;"
assert 1 "20==20;"
assert 0 "20!=20;"
assert 0 "20<5;"
assert 1 "2<5;"
assert 1 "20<=20;"
assert 1 "20<=21;"
assert 3 "a = 1; a + 2;"
assert 5 "a = 1; 2 + 2 + a;"
assert 10 "a = 6; b=4; a + b;"
assert 12 "a = 6; b=2; a * b;"
assert 12 "z = 6; u=2; u * z;"
assert 12 "zz = 6; uv=2; uv * zz;"
assert 19 "zz = 6; uv=zz /2; uv * zz + 1;"
assert 42 "abcdef = 42; abcdef;"
assert 13 "zz = 6; uv=zz /2; zz=4; uv * zz + 1;"

echo OK
