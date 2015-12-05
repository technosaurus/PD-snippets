#!/bin/ash

nth(){ #prints the nth($2) token in string($1) separated by delimiter($3)
[ $# -eq 3 ] || return 1
local String="$1" i="$2" IFS="$3"
set $String
eval printf \${$i}
}


nth_var (){ #sets var($4) to nth($2) token in string($1) separated by ($3)
[ $# -eq 4 ] || return 1
local String="$1" i="$2" IFS="$3" Var="$4"
set $String
eval $Var=\"\${$i}\"
}

sh_test(){
CorrectIFS="$IFS"
echo "nth() got:"
nth "a,b,c,d,e,f,g,h" 3 ,
echo "$String$IFS$i"
echo "should be:
c
$CorrectIFS"

echo "nth_var() got:"
nth_var "a,b,c,d,e,f,g,h" 4 , A
echo "$A"
echo "should be:
d
$CorrectIFS"

}

$@