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

parse_desktop(){
	#standard keys - damn there are a lot of useless keys to clobber
	Name= GenericName= Comment= Exec= Icon= Terminal= TryExec= Type= MimeType=
	Categories= Actions= OnlyShowIn= NotShowIn= Version= Hidden= Actions= Path=
	Implements= Keywords= StartupNotify= StartupWMClass= DBusActivatable= URL=
	#kde
	ServiceTypes= DocPath= InitialPreference= FSType= MountPoint= ReadOnly=
	UnmountIcon= Dev=
	#deprecated keys intentionally omitted
	#Encoding= MiniIcon= TerminalOptions= Protocols= Extensions= BinaryPattern=
	#MapNotify= SwallowTitle= SwallowExec= SortOrder= FilePattern= 
	[ -f "$1" ] || return 1
	local IFS="="
	while read Var Val; do
	case "$Var" in
	"[Desktop Entry]")continue;;
	[*)break;; #don't handle groups/actions
	X-*): todo; continue;;
	*"[$LANG]"|*"${LANG%@*}"|*"${LANG%_*}")
		Var="${Var%[*}"
		eval $Var=\"\$Val\";;
	*]|"")continue;;
	*)eval $Var=\"\$Val\";;
	esac
	done < "$1"
}

sh_test(){
	local CorrectIFS="$IFS"
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


for desktopFile in /usr/share/applications/*.desktop; do
echo "
  in native language:
"
parse_desktop "$desktopFile"
echo "$Name|$GenericName|$Comment|$Exec|$Icon|$Terminal|$TryExec|$Type|$MimeType|$Categories|$Actions"

echo "
  en espanol:
"
LANG=es parse_desktop "$desktopFile"
echo "$Name|$GenericName|$Comment|$Exec|$Icon|$Terminal|$TryExec|$Type|$MimeType|$Categories|$Actions"
break #only test 1 file
done
}

$@