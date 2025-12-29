#!/bin/bash

# uncomment for memory testing
#VALGRIND='valgrind'

cleanup(){
    [ -f output.txt  ] && rm -f output.txt
    [ -f output.txt1 ] && rm -f output.txt1
    [ -f output.txt2 ] && rm -f output.txt2
    [ -f "$PWD/test_profile" ] && rm -f "$PWD/test_profile"
}

verify(){
    local received="$1"
    local expected="$2"

    echo "received:" "$(< $received)"
    echo "expected:" "$2"
    if diff "$received" - <<< "$expected"; then
        echo "[PASSED]"
        return 0
    else
        echo "[FAILED]"
        return 1
    fi
}

ERROR=0
cleanup

echo "truncate tests --------------------------------------"
echo "restrict length to less"
NUM=$($VALGRIND ./igloo -t 16 -q "cat back z" <<< "0123456789abcdefghij" | wc -m)
if [ "$NUM" -ne "17" ]; then
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
else
    echo "[PASSED]"
fi
echo ""

echo "restrict length to same"
NUM=$($VALGRIND ./igloo -t 20 -q "cat back z" <<< "0123456789abcdefghij" | wc -m)
if [ "$NUM" -ne "21" ]; then
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
else
    echo "[PASSED]"
fi
echo ""

echo "restrict length to more"
NUM=$($VALGRIND ./igloo -t 24 -q "cat back z" <<< "0123456789abcdefghij" | wc -m)
if [ "$NUM" -ne "22" ]; then
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
else
    echo "[PASSED]"
fi
echo ""

echo "no length restriction"
NUM=$($VALGRIND ./igloo -t 0 -q "cat back z"<<< "0123456789abcdefghij" | wc -m)
if [ "$NUM" -ne "22" ]; then
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
else
    echo "[PASSED]"
fi
echo ""

echo "multi-password tests --------------------------------"
echo "restrict to 1 with output"
rm -f output.txt
$VALGRIND ./igloo -o output.txt -p 1 -q "cat back z" <<< "igloo"
echo "" >> output.txt
verify output.txt "iglooz"
ERROR=$(($ERROR + $?))
echo ""

echo "restrict to 2 with output"
rm -f output.txt1; rm -f output.txt2
echo -e "igloo\nsnow" | $VALGRIND ./igloo -o output.txt -p 2 -q "cat back z"
echo "" >> output.txt1; echo "" >> output.txt2
verify output.txt1 "iglooz" >/dev/null
RES1=$?
verify output.txt2 "snowz" >/dev/null
RES2=$?
if [ "${RES1}" -eq "0" ] && [ "${RES2}" -eq "0" ]; then
    echo "[PASSED]"
else
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
fi
rm -f output.txt1; rm -f output.txt2
echo ""

echo "save/load profile tests -----------------------------"
echo "save test"
rm -f "$PWD/test_profile"
$VALGRIND ./igloo -q -s "$PWD/test_profile" "cat front igloo" "rev" <<< "igloo"
$VALGRIND ./igloo -q -l "$PWD/test_profile" <<< "igloo" > output.txt
verify output.txt "oolgioolgi"
ERROR=$(($ERROR + $?))
rm -f "$PWD/test_profile"
echo ""

echo "load test"
if [ -f "$HOME/.igloo/default" ]; then
    echo -e 'igloo\nsalt' | $VALGRIND ./igloo -q -l default > output.txt
    verify output.txt 'Rc&WiN{P^77i^veL6X@Lr"^m}5r/"(gG'
    ERROR=$(($ERROR + $?))
else
    echo "[FAILED]"
    ERROR=$(($ERROR + 1))
fi
echo ""

echo "CAT tests -------------------------------------------"
echo "cat front"
$VALGRIND ./igloo -q "cat front front" <<< "igloo" > output.txt
verify output.txt "frontigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "cat back"
$VALGRIND ./igloo -q "cat back back" <<< "igloo" > output.txt
verify output.txt "iglooback"
ERROR=$(($ERROR + $?))
echo ""

echo "cat 17 times" # to test memory allocation
$VALGRIND ./igloo -q \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" \
    "cat back back" <<< "igloo" > output.txt
verify output.txt "igloobackbackbackbackbackbackbackbackbackbackbackbackbackbackbackbackback"
ERROR=$(($ERROR + $?))
echo ""

echo "SUB tests -------------------------------------------"
echo "smaller substitute"
$VALGRIND ./igloo -q "sub help me 0" <<< "help, help I am drowning" > output.txt
verify output.txt "me, me I am drowning"
ERROR=$(($ERROR + $?))
echo ""

echo "bigger substitute"
$VALGRIND ./igloo -q "sub me help 0" <<< "me, me I am drowning" > output.txt
verify output.txt "help, help I am drowning"
ERROR=$(($ERROR + $?))
echo ""

echo "single substitute"
$VALGRIND ./igloo -q 'sub me\, help 1' <<< "me, me I am drowning" > output.txt
verify output.txt "help me I am drowning"
ERROR=$(($ERROR + $?))
echo ""

echo "no substitute"
$VALGRIND ./igloo -q "sub helpme help 1" <<< "help me I am drowning" > output.txt
verify output.txt "help me I am drowning"
ERROR=$(($ERROR + $?))
echo ""

echo "CUT tests -------------------------------------------"
echo "cut at beginning"
$VALGRIND ./igloo -q "cut 1" <<< "igloo" > output.txt
verify output.txt "glooi"
ERROR=$(($ERROR + $?))
echo ""

echo "cut at end"
$VALGRIND ./igloo -q "cut 4" <<< "igloo" > output.txt
verify output.txt "oiglo"
ERROR=$(($ERROR + $?))
echo ""

echo "cut at middle"
$VALGRIND ./igloo -q "cut 0" <<< "igloo" > output.txt
verify output.txt "looig"
ERROR=$(($ERROR + $?))
echo ""

echo "REV tests -------------------------------------------"
echo 'reverse odd # of characters'
$VALGRIND ./igloo -q "rev" <<< "igloo" > output.txt
verify output.txt "oolgi"
ERROR=$(($ERROR + $?))
echo ""

echo 'reverse even # of characters'
$VALGRIND ./igloo -q "rev" <<< "snow" > output.txt
verify output.txt "wons"
ERROR=$(($ERROR + $?))
echo ""

echo "XOR tests -------------------------------------------"
echo "xor with shorter string"
$VALGRIND ./igloo -q 'xor \x01\x01\x01\x01' <<< "igloo" > output.txt
verify output.txt "hfmno"
ERROR=$(($ERROR + $?))
echo ""

echo "xor with longer string"
$VALGRIND ./igloo -q 'xor \x01\x01\x01\x01\x01\x01' <<< "igloo" > output.txt
verify output.txt "hfmnn"
ERROR=$(($ERROR + $?))
echo ""

echo "REPLACE tests ---------------------------------------"
echo "replace from beginning, not exceeding end"
$VALGRIND ./igloo -q "replace snow 0" <<< "igloo" > output.txt
verify output.txt "snowo"
ERROR=$(($ERROR + $?))
echo ""

echo "replace from beginning, exceeding end"
$VALGRIND ./igloo -q "replace snowing 0" <<< "igloo" > output.txt
verify output.txt "snowing"
ERROR=$(($ERROR + $?))
echo ""

echo "replace at end"
$VALGRIND ./igloo -q "replace snow 5" <<< "igloo" > output.txt
verify output.txt "igloosnow"
ERROR=$(($ERROR + $?))
echo ""

echo "SHUFFLE tests ---------------------------------------"
echo "shuffle 1, even"
$VALGRIND ./igloo -q "shuffle 1" <<< "12345678" > output.txt
verify output.txt "15263748"
ERROR=$(($ERROR + $?))
echo ""

echo "shuffle 2, even"
$VALGRIND ./igloo -q "shuffle 2" <<< "12345678" > output.txt
verify output.txt "13572468"
ERROR=$(($ERROR + $?))
echo ""

echo "shuffle 3, even"
$VALGRIND ./igloo -q "shuffle 3" <<< "12345678" > output.txt
verify output.txt "12345678"
ERROR=$(($ERROR + $?))
echo ""

echo "shuffle 1, odd"
$VALGRIND ./igloo -q "shuffle 1" <<< "123456789" > output.txt
verify output.txt "162738495"
ERROR=$(($ERROR + $?))
echo ""

echo "shuffle 2, odd"
$VALGRIND ./igloo -q "shuffle 2" <<< "123456789" > output.txt
verify output.txt "186429753"
ERROR=$(($ERROR + $?))
echo ""

echo "shuffle 3, odd"
$VALGRIND ./igloo -q "shuffle 3" <<< "123456789" > output.txt
verify output.txt "198765432"
ERROR=$(($ERROR + $?))
echo ""

echo "CCIPHER tests ---------------------------------------"
echo "ccipher 0"
$VALGRIND ./igloo -q "ccipher 0" <<< "igloo" > output.txt
verify output.txt "igloo"
ERROR=$(($ERROR + $?))
echo ""

echo "ccipher 1"
$VALGRIND ./igloo -q "ccipher 1" <<< "igloo" > output.txt
verify output.txt "jhmpp"
ERROR=$(($ERROR + $?))
echo ""

echo "ccipher -1"
$VALGRIND ./igloo -q "ccipher -1" <<< "igloo" > output.txt
verify output.txt "hfknn"
ERROR=$(($ERROR + $?))
echo ""

echo "ccipher 257"
$VALGRIND ./igloo -q "ccipher 257" <<< "igloo" > output.txt
verify output.txt "jhmpp"
ERROR=$(($ERROR + $?))
echo ""

echo "RCIPHER tests ---------------------------------------"
echo "rcipher 1"
$VALGRIND ./igloo -q "rcipher 1" <<< "build an igloo from ice and snow" > output.txt
verify output.txt "build an igloo from ice and snow"
ERROR=$(($ERROR + $?))
echo ""

echo "rcipher 2"
$VALGRIND ./igloo -q "rcipher 2" <<< "build an igloo from ice and snow" > output.txt
verify output.txt "bida go rmieadsoul nilofo c n nw"
ERROR=$(($ERROR + $?))
echo ""

echo "rcipher 3"
$VALGRIND ./igloo -q "rcipher 3" <<< "build an igloo from ice and snow" > output.txt
verify output.txt "bd oriasul nilofo c n nwiag medo"
ERROR=$(($ERROR + $?))
echo ""

echo "rcipher 4"
$VALGRIND ./igloo -q "rcipher 4" <<< "build an igloo from ice and snow" > output.txt
verify output.txt "baomaou nloo  nnwid g riedslifc "
ERROR=$(($ERROR + $?))
echo ""

echo "rcipher 5"
$VALGRIND ./igloo -q "rcipher 5" <<< "build an igloo from ice and snow" > output.txt
verify output.txt "b raunifo nwiag medol lo c ndois"
ERROR=$(($ERROR + $?))
echo ""

echo "COMPLEMENT tests ------------------------------------"
echo "complement"
$VALGRIND ./igloo -q "complement" <<< '\x96\x98\x93\x90\x90' > output.txt
verify output.txt "igloo"
ERROR=$(($ERROR + $?))
echo ""

echo "GENXOR tests ----------------------------------------"
echo "genxor, dynamic seed"
$VALGRIND ./igloo -q "genxor 0" 'xor \x21\x6f\xbb\xfd\x97' <<< "igloo" > output.txt
verify output.txt "igloo"
ERROR=$(($ERROR + $?))
echo ""

echo "genxor, dynamic seed extended"
$VALGRIND ./igloo -q "genxor 0" 'xor \xa3\xad\x46\x60\x25\x85\x5e\xce\xd7' <<< "igloosnow" > output.txt
verify output.txt "igloosnow"
ERROR=$(($ERROR + $?))
echo ""

echo "genxor, seed 1"
$VALGRIND ./igloo -q "genxor 1" 'xor \x7f\xd0\xc5\xcf\x32' <<< "igloo" > output.txt
verify output.txt "igloo"
ERROR=$(($ERROR + $?))
echo ""

echo "genxor, seed 1 extended"
$VALGRIND ./igloo -q "genxor 1" 'xor \x7f\xd0\xc5\xcf\x32\x41\x42\xbf\x57' <<< "igloosnow" > output.txt
verify output.txt "igloosnow"
ERROR=$(($ERROR + $?))
echo ""

echo "PASSWORDIFY tests -----------------------------------"
echo "passwordify, dynamic seed"
$VALGRIND ./igloo -q 'passwordify 0 \ ' <<< "igloo" > output.txt
verify output.txt "HN7G>"
ERROR=$(($ERROR + $?))
echo ""

echo "passwordify, dynamic seed extended"
$VALGRIND ./igloo -q 'passwordify 0 \ ' <<< "igloosnow" > output.txt
verify output.txt "g!)zqwboD"
ERROR=$(($ERROR + $?))
echo ""

echo "passwordify, seed 1"
$VALGRIND ./igloo -q 'passwordify 1 \ ' <<< "igloo" > output.txt
verify output.txt "B04~g"
ERROR=$(($ERROR + $?))
echo ""

echo "passwordify, seed 1 extended"
$VALGRIND ./igloo -q 'passwordify 1 \ ' <<< "igloosnow" > output.txt
verify output.txt "B04~g=O=("
ERROR=$(($ERROR + $?))
echo ""

echo "passwordify, only 'a' allowed"
$VALGRIND ./igloo -q 'passwordify 0 bcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-_=+[{]}\\|'\''";:/?.>\,<' <<< "igloosnow" > output.txt
verify output.txt "aaaaaaaaa"
ERROR=$(($ERROR + $?))
echo ""

echo "FORK tests ------------------------------------------"
echo "1 fork"
$VALGRIND ./igloo -q fork 'join cat\ front\ FORK1' <<< "igloo" > output.txt
verify output.txt "iglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "2 fork"
$VALGRIND ./igloo -q fork fork 'join cat\ front\ FORK2' 'join cat\ front\ FORK1' <<< "igloo" > output.txt
verify output.txt "iglooiglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "9 fork"
$VALGRIND ./igloo -q fork fork fork fork fork fork fork fork fork \
    'join cat\ front\ FORK9' \
    'join cat\ front\ FORK8' \
    'join cat\ front\ FORK7' \
    'join cat\ front\ FORK6' \
    'join cat\ front\ FORK5' \
    'join cat\ front\ FORK4' \
    'join cat\ front\ FORK3' \
    'join cat\ front\ FORK2' \
    'join cat\ front\ FORK1' <<< "igloo" > output.txt
verify output.txt "iglooiglooiglooiglooiglooiglooiglooiglooiglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "SUBSTR tests ----------------------------------------"
echo "small substr"
$VALGRIND ./igloo -q "substr 0 3" 'join cat\ front\ SUBSTR1' <<< "igloo" > output.txt
verify output.txt "igligloo"
ERROR=$(($ERROR + $?))
echo ""

echo "big substr"
$VALGRIND ./igloo -q "substr 0 5" 'join cat\ front\ SUBSTR1' <<< "igloo" > output.txt
verify output.txt "iglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "JOIN tests ------------------------------------------"
echo "join with cat"
$VALGRIND ./igloo -q fork 'join cat\ front\ FORK1' <<< "igloo" > output.txt
verify output.txt "iglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "join with sub"
$VALGRIND ./igloo -q fork 'join sub\ o\ FORK1\ 0' <<< "igloo" > output.txt
verify output.txt "igliglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "join with xor"
$VALGRIND ./igloo -q fork 'join xor\ FORK1' 'xor \x69\x67\x6c\x6f\x6f' <<< "igloo" > output.txt
verify output.txt "igloo"
ERROR=$(($ERROR + $?))
echo ""

echo "join with replace"
$VALGRIND ./igloo -q fork 'join replace\ FORK1\ 1' <<< "igloo" > output.txt
verify output.txt "iigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "ITERATE tests ---------------------------------------"
echo "iterate 1 time (i.e., dont repeat)"
$VALGRIND ./igloo -q "iterate 1" "cat front a" "iterate_end" <<< "igloo" > output.txt
verify output.txt "aigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "iterate 5 times"
$VALGRIND ./igloo -q "iterate 5" "cat front a" "iterate_end" <<< "igloo" > output.txt
verify output.txt "aaaaaigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "nested iterate"
$VALGRIND ./igloo -q "iterate 2" "iterate 5" "cat front a" "iterate_end" "iterate_end" <<< "igloo" > output.txt
verify output.txt "aaaaaaaaaaigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "JOIN and ITERATE tests ------------------------------"
echo "join and iterate 2 times"
$VALGRIND ./igloo -q "iterate 2" "fork" "cat front a" 'join cat\ front\ FORK1' "iterate_end" <<< "igloo" > output.txt
verify output.txt "aaiglooiglooaiglooigloo"
ERROR=$(($ERROR + $?))
echo ""

echo "ERRORS: $ERROR"
cleanup
