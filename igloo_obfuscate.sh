HASH_ALGS=()
BEFUDDLE_ALGS=()

hasher(){
    TMP="$1"
    for alg in "${HASH_ALGS[@]}"; do
        TMP=`$alg <<< "$TMP"`
    done
    echo -n "$TMP"
}

befuddler(){
    TMP="$1"
    for alg in "${BEFUDDLE_ALGS[@]}"; do
        echo bufuddle
    done

    hasher "$TMP"
}

help_befuddler(){
    echo
}
