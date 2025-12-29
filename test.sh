#!/bin/bash

# uncomment for memory testing
#VALGRIND='valgrind'

ERROR=0
ERROR_NAMES=""

cleanup(){
    [ -f output.txt  ] && rm -f output.txt
    [ -f output.txt1 ] && rm -f output.txt1
    [ -f output.txt2 ] && rm -f output.txt2
    [ -f "$PWD/test_profile" ] && rm -f "$PWD/test_profile"
}

save_error(){
    local error_val="$1"

    if [ $error_val -eq 0 ]; then
        echo "[PASSED]"
    else
        echo "[FAILED]"
        ERROR=$(($ERROR + 1))
        ERROR_NAMES+="\n$TEST_NAME"
    fi
}

check_file_error(){
    local file="$1"
    local expected="$2"

    echo "$TEST_NAME"

    echo "received:" "$(< "$file")"
    echo "expected:" "$expected"
    diff "$file" - <<< "$expected"
    save_error $?
}

check_value_error(){
    local num="$1"
    local expected="$2"

    echo "$TEST_NAME"

    echo "received:" "$num"
    echo "expected:" "$expected"
    [[ "$num" -eq "$expected" ]]
    save_error $?
}

verify(){
    local input="$1"
    local alg="$2"
    local expected="$3"

    [ -f output.txt ] && rm -f output.txt
    echo "$input" | $VALGRIND ./igloo -q "$alg" > output.txt
    check_file_error "output.txt" "$expected"
}

verify_count(){
    local input="$1"
    local truncate="$2"
    local expected="$3"

    local num=0

    num=$($VALGRIND ./igloo -t "$truncate" -q "cat back z" <<< "$input" | wc -m)
    check_value_error $num $expected
}

verify_hash(){
    local input="$1"
    local alg="$2"
    local expected="$3"

    [ -f output.txt ] && rm -f output.txt
    echo "$input" | $VALGRIND ./igloo -q "$alg" | od -tx1 -w1024 -An - | sed 's/ //g' - > output.txt
    check_file_error "output.txt" "$expected"
}

cleanup

echo "truncate tests --------------------------------------"
TEST_NAME="restrict length to less"
verify_count "0123456789abcdefghij" "16" \
             "17"
echo ""

TEST_NAME="restrict length to same"
verify_count "0123456789abcdefghij" "20" \
             "21"
echo ""

TEST_NAME="restrict length to more"
verify_count "0123456789abcdefghij" "24" \
             "22"
echo ""

TEST_NAME="no length restriction"
verify_count "0123456789abcdefghij" "0" \
             "22"
echo ""

echo "multi-password tests --------------------------------"
TEST_NAME="restrict to 1 with output"
[ -f output.txt ] && rm -f output.txt
$VALGRIND ./igloo -o output.txt -p 1 -q "cat back z" <<< "igloo"
echo "" >> output.txt
check_file_error "output.txt" "iglooz"
echo ""

TEST_NAME="restrict to 2 with output"
[ -f output.txt1 ] && rm -f output.txt1
[ -f output.txt2 ] && rm -f output.txt2
echo -e "igloo\nsnow" | $VALGRIND ./igloo -o output.txt -p 2 -q "cat back z"
echo "" >> output.txt1; echo "" >> output.txt2
check_file_error "output.txt1" "iglooz"
RES1=$?
check_file_error "output.txt2" "snowz"
RES2=$?
if [ "${RES1}" -eq "0" ] && [ "${RES2}" -eq "0" ]; then
    save_error 0
else
    save_error 1
fi
[ -f output.txt1 ] && rm -f output.txt1
[ -f output.txt2 ] && rm -f output.txt2
echo ""

echo "save/load profile tests -----------------------------"
TEST_NAME="save test"
[ -f "$PWD/test_profile" ] && rm -f "$PWD/test_profile"
[ -f output.txt          ] && rm -f output.txt
$VALGRIND ./igloo -q -s "$PWD/test_profile" "cat front igloo" "rev" <<< "igloo"
$VALGRIND ./igloo -q -l "$PWD/test_profile" <<< "igloo" > output.txt
check_file_error "output.txt" "oolgioolgi"
[ -f "$PWD/test_profile" ] && rm -f "$PWD/test_profile"
echo ""

TEST_NAME="load test"
if [ -f "$HOME/.igloo/default" ]; then
    echo -e 'igloo\nsalt' | $VALGRIND ./igloo -q -l default > output.txt
    check_file_error "output.txt" 'Rc&WiN{P^77i^veL6X@Lr"^m}5r/"(gG'
fi
echo ""

echo "CAT tests -------------------------------------------"
TEST_NAME="cat front"
verify "igloo" "cat front front" \
       "frontigloo"
echo ""

TEST_NAME="cat back"
verify "igloo" "cat back back" \
       "iglooback"
echo ""

TEST_NAME="cat 17 times" # to test memory allocation
verify "igloo" "cat back back, cat back back, cat back back, cat back back,
                cat back back, cat back back, cat back back, cat back back,
                cat back back, cat back back, cat back back, cat back back,
                cat back back, cat back back, cat back back, cat back back,
                cat back back" \
       "igloobackbackbackbackbackbackbackbackbackbackbackbackbackbackbackbackback"
echo ""

echo "SUB tests -------------------------------------------"
TEST_NAME="smaller substitute"
verify "help, help I am drowning" "sub help me 0" \
       "me, me I am drowning"
echo ""

TEST_NAME="bigger substitute"
verify "me, me I am drowning" "sub me help 0" \
       "help, help I am drowning"
echo ""

TEST_NAME="single substitute"
verify "me, me I am drowning" 'sub me\, help 1' \
       "help me I am drowning"
echo ""

TEST_NAME="no substitute"
verify "help me I am drowning" 'sub helpme help 1' \
       "help me I am drowning"
echo ""

echo "CUT tests -------------------------------------------"
TEST_NAME="cut at beginning"
verify "igloo" "cut 1" \
       "glooi"
echo ""

TEST_NAME="cut at end"
verify "igloo" "cut 4" \
       "oiglo"
echo ""

TEST_NAME="cut at middle"
verify "igloo" "cut 0" \
       "looig"
echo ""

echo "REV tests -------------------------------------------"
TEST_NAME='reverse odd # of characters'
verify "igloo" "rev" \
       "oolgi"
echo ""

TEST_NAME='reverse even # of characters'
verify "snow" "rev" \
       "wons"
echo ""

echo "XOR tests -------------------------------------------"
TEST_NAME="xor with shorter string"
verify "igloo" 'xor \x01\x01\x01\x01' \
       "hfmno"
echo ""

TEST_NAME="xor with longer string"
verify "igloo" 'xor \x01\x01\x01\x01\x01\x01' \
       "hfmnn"
echo ""

echo "REPLACE tests ---------------------------------------"
TEST_NAME="replace from beginning, not exceeding end"
verify "igloo" "replace snow 0" \
       "snowo"
echo ""

TEST_NAME="replace from beginning, exceeding end"
verify "igloo" "replace snowing 0" \
       "snowing"
echo ""

TEST_NAME="replace at end"
verify "igloo" "replace snow 5" \
       "igloosnow"
echo ""

echo "SHUFFLE tests ---------------------------------------"
TEST_NAME="shuffle 1, even"
verify "12345678" "shuffle 1" \
       "15263748"
echo ""

TEST_NAME="shuffle 2, even"
verify "12345678" "shuffle 2" \
       "13572468"
echo ""

TEST_NAME="shuffle 3, even"
verify "12345678" "shuffle 3" \
       "12345678"
echo ""

TEST_NAME="shuffle 1, odd"
verify "123456789" "shuffle 1" \
       "162738495"
echo ""

TEST_NAME="shuffle 2, odd"
verify "123456789" "shuffle 2" \
       "186429753"
echo ""

TEST_NAME="shuffle 3, odd"
verify "123456789" "shuffle 3" \
       "198765432"
echo ""

echo "CCIPHER tests ---------------------------------------"
TEST_NAME="ccipher 0"
verify "igloo" "ccipher 0" \
       "igloo"
echo ""

TEST_NAME="ccipher 1"
verify "igloo" "ccipher 1" \
       "jhmpp"
echo ""

TEST_NAME="ccipher -1"
verify "igloo" "ccipher -1" \
       "hfknn"
echo ""

TEST_NAME="ccipher 257"
verify "igloo" "ccipher 257" \
       "jhmpp"
echo ""

echo "RCIPHER tests ---------------------------------------"
TEST_NAME="rcipher 1"
verify "build an igloo from ice and snow" "rcipher 1" \
       "build an igloo from ice and snow"
echo ""

TEST_NAME="rcipher 2"
verify "build an igloo from ice and snow" "rcipher 2" \
       "bida go rmieadsoul nilofo c n nw"
echo ""

TEST_NAME="rcipher 3"
verify "build an igloo from ice and snow" "rcipher 3" \
       "bd oriasul nilofo c n nwiag medo"
echo ""

TEST_NAME="rcipher 4"
verify "build an igloo from ice and snow" "rcipher 4" \
       "baomaou nloo  nnwid g riedslifc "
echo ""

TEST_NAME="rcipher 5"
verify "build an igloo from ice and snow" "rcipher 5" \
       "b raunifo nwiag medol lo c ndois"
echo ""

echo "COMPLEMENT tests ------------------------------------"
TEST_NAME="complement"
verify '\x96\x98\x93\x90\x90' "complement" \
       "igloo"
echo ""

echo "GENXOR tests ----------------------------------------"
TEST_NAME="genxor, dynamic seed"
verify "igloo" 'genxor 0, xor \x21\x6f\xbb\xfd\x97' \
       "igloo"
echo ""

TEST_NAME="genxor, dynamic seed extended"
verify "igloosnow" 'genxor 0, xor \xa3\xad\x46\x60\x25\x85\x5e\xce\xd7' \
       "igloosnow"
echo ""

TEST_NAME="genxor, seed 1"
verify "igloo" 'genxor 1, xor \x7f\xd0\xc5\xcf\x32' \
       "igloo"
echo ""

TEST_NAME="genxor, seed 1 extended"
verify "igloosnow" 'genxor 1, xor \x7f\xd0\xc5\xcf\x32\x41\x42\xbf\x57' \
       "igloosnow"
echo ""

echo "PASSWORDIFY tests -----------------------------------"
TEST_NAME="passwordify, dynamic seed"
verify "igloo" 'passwordify 0 \ ' \
       "HN7G>"
echo ""

TEST_NAME="passwordify, dynamic seed extended"
verify "igloosnow" 'passwordify 0 \ ' \
       "g!)zqwboD"
echo ""

TEST_NAME="passwordify, seed 1"
verify "igloo" 'passwordify 1 \ ' \
       "B04~g"
echo ""

TEST_NAME="passwordify, seed 1 extended"
verify "igloosnow" 'passwordify 1 \ ' \
       "B04~g=O=("
echo ""

TEST_NAME="passwordify, only 'a' allowed"
verify "igloosnow" 'passwordify 0 bcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-_=+[{]}\\|'\''";:/?.>\,<' \
       "aaaaaaaaa"
echo ""

echo "FORK tests ------------------------------------------"
TEST_NAME="1 fork"
verify "igloo" 'fork, join cat\ front\ FORK1' \
       "iglooigloo"
echo ""

TEST_NAME="2 fork"
verify "igloo" 'fork, fork, join cat\ front\ FORK2, join cat\ front\ FORK1' \
       "iglooiglooigloo"
echo ""

TEST_NAME="9 fork" # to test memory allocation
verify "igloo" 'fork, fork, fork, fork, fork, fork, fork, fork, fork,
                join cat\ front\ FORK9,
                join cat\ front\ FORK8,
                join cat\ front\ FORK7,
                join cat\ front\ FORK6,
                join cat\ front\ FORK5,
                join cat\ front\ FORK4,
                join cat\ front\ FORK3,
                join cat\ front\ FORK2,
                join cat\ front\ FORK1' \
       "iglooiglooiglooiglooiglooiglooiglooiglooiglooigloo"
echo ""

echo "SUBSTR tests ----------------------------------------"
TEST_NAME="small substr"
verify "igloo" 'substr 0 3, join cat\ front\ SUBSTR1' \
       "igligloo"
echo ""

TEST_NAME="big substr"
verify "igloo" 'substr 0 5, join cat\ front\ SUBSTR1' \
       "iglooigloo"
echo ""

echo "JOIN tests ------------------------------------------"
TEST_NAME="join with cat"
verify "igloo" 'fork, join cat\ front\ FORK1' \
       "iglooigloo"
echo ""

TEST_NAME="join with sub"
verify "igloo" 'fork, join sub\ o\ FORK1\ 0' \
       "igliglooigloo"
echo ""

TEST_NAME="join with xor"
verify "igloo" 'fork, join xor\ FORK1, xor \x69\x67\x6c\x6f\x6f' \
       "igloo"
echo ""

TEST_NAME="join with replace"
verify "igloo" 'fork, join replace\ FORK1\ 1' \
       "iigloo"
echo ""

echo "ITERATE tests ---------------------------------------"
TEST_NAME="iterate 1 time (i.e., dont repeat)"
verify "igloo" "iterate 1, cat front a, iterate_end" \
       "aigloo"
echo ""

TEST_NAME="iterate 5 times"
verify "igloo" "iterate 5, cat front a, iterate_end" \
       "aaaaaigloo"
echo ""

TEST_NAME="nested iterate"
verify "igloo" "iterate 2, iterate 5, cat front a, iterate_end, iterate_end" \
       "aaaaaaaaaaigloo"
echo ""

TEST_NAME="5 iterate's" # to test memory allocation
verify "igloo" "iterate 2, iterate 2, iterate 2, iterate 2, iterate 2, \
                cat front a, \
                iterate_end, iterate_end, iterate_end, iterate_end, iterate_end" \
       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaigloo"
echo ""

echo "JOIN and ITERATE tests ------------------------------"
TEST_NAME="join and iterate 2 times"
verify "igloo" 'iterate 2, fork, cat front a, join cat\ front\ FORK1, iterate_end' \
       "aaiglooiglooaiglooigloo"
echo ""

echo "HASH tests ------------------------------------------"
TEST_NAME="sha2-256 test"
verify_hash "igloo" "sha2-256" \
            "01b1539b30a2d85b25b95ec506a65dc894a94b5c5c60dc14d8c968a5a88e6aee0a"
echo ""

TEST_NAME="sha2-512 test"
verify_hash "igloo" "sha2-512" \
            "38b5a043ab36be39cd4c7c9b0f77dde3280dd823654dc64a226a7d48b6a460933cee78d87d45cd2d5b9450c07bb36c286498db559779451fe2c0c714c1cd1d7b0a"
echo ""

TEST_NAME="sha3-256 test"
verify_hash "igloo" "sha3-256" \
            "fc9342df8b0e46d262ebc756b7a00fe3272dc008f979712e3888113aa50c39c00a"
echo ""

TEST_NAME="sha3-512 test"
verify_hash "igloo" "sha3-512" \
            "f86598196d636286da077b8f9310f817f1b90a9882e3ba62039d0d2fe3a099ef3473e19d6f807eac210f03b7047c68f8c0903042d53552ee22c87b76f66a01710a"
echo ""

TEST_NAME="blake2b test (1 byte)"
verify_hash "igloo" "blake2b 1" \
            "9d0a"
echo ""

TEST_NAME="blake2b test (64 bytes)"
verify_hash "igloo" "blake2b 64" \
            "d6e46c104e35834919ab9ba83b2b7b75fc7705b44a48b8e43400a7513970b3180fef90c3c1c7bd3d2fa3c852d5155fb1c14ea3dbf31ac7e1800dd4d0ddb1be840a"
echo ""

TEST_NAME="blake2s test (1 byte)"
verify_hash "igloo" "blake2s 1" \
            "520a"
echo ""

TEST_NAME="blake2s test (32 bytes)"
verify_hash "igloo" "blake2s 32" \
            "173e7f73fa204f2bf6ad0588403f608f7231d578a21e1b3e63ef1be37ceb2f6f0a"
echo ""

TEST_NAME="shake128 test (1 byte)"
verify_hash "igloo" "shake128 1" \
            "130a"
echo ""

TEST_NAME="shake128 test (64 bytes)"
verify_hash "igloo" "shake128 64" \
            "13386a421850eacf32b6b32e74d7e155fa0493c75cefa5438b435c73f6f3d37408e18ba08b2c81c26a0fc2ce804360fcbc3dc0b3284ee0c5e3e2f0fd601c81b60a"
echo ""

TEST_NAME="shake256 test (1 byte)"
verify_hash "igloo" "shake256 1" \
            "3d0a"
echo ""

TEST_NAME="shake256 test (64 bytes)"
verify_hash "igloo" "shake256 64" \
            "3d55c0450753f20e1a154450d58d22326006e5efc9024ed5fa03e0df861e06dcd8c101d5a6035a9adbb354d73cd465d89cfb33546e77a04ae7ac63ea7ab2149a0a"
echo ""

echo "ERRORS: $ERROR"
echo -e "$ERROR_NAMES"
cleanup
