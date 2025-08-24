#!/bin/bash

# navigate to the directory containing igloo.sh
SCRIPT_DIR=`readlink -f "$0"`
SCRIPT_DIR=`dirname $SCRIPT_DIR`
cd "$SCRIPT_DIR"

# implements the hasher and befuddler functions
source igloo_obfuscate.sh

# location to store and access profiles, this may be specified beforehand
PROFILE_DIR=${PROFILE_DIR:-".profiles"}
if [ ! -d "$PROFILE_DIR" ]; then mkdir "$PROFILE_DIR"; fi

generate_profile(){
    cd "$PROFILE_DIR"

    echo "Hash:" > $PROFILE_OUTPUT
    for alg in "${HASH_ALGS[@]}"; do
        echo "$alg" >> "$PROFILE_OUTPUT"
    done

    echo "Befuddle:" >> $PROFILE_OUTPUT
    for alg in "${BEFUDDLE_ALGS[@]}"; do
        echo "$alg" >> "$PROFILE_OUTPUT"
    done

    cd - > /dev/null
}

parse_profile(){
    cd "$PROFILE_DIR"

    while IFS= read -r line; do
        case "$line" in
            Hash:)
                type="hash"
                continue
                ;;

            Befuddle:)
                type="befuddle"
                continue
                ;;

            *)  ;;
        esac

        if [[ $type = "hash" ]]; then
            HASH_ALGS+=("$line")
        elif [[ $type == "befuddle" ]]; then
            # this line interprets escape sequences. This allows things like
            # \xFF (character 255) to be used in befuddle commands
            alg=`printf "$line"`
            BEFUDDLE_ALGS+=("$alg")
        else
            continue
        fi
    done < "$1"

    cd - > /dev/null
}

main(){
    if [ -n "$PROFILE_OUTPUT" ]; then
        generate_profile
    fi

    if [ ! -n "$1" ]; then
        if [[ $SILENT != true ]]; then
            echo -n "Input password: "
        fi
        read PASSWORD
    fi

    for PASSWORD in "$@"; do
        PASSWORD=`hasher "$PASSWORD"`
        PASSWORD=`befuddler "$PASSWORD"`
        echo "$PASSWORD"
    done
}

help_help(){
    echo "igloo [flags] password"
    echo ""
    echo "A tool to obscure passwords"
    echo "\tSet a sequence of hash algorithms and character permutations to alter an input"
    echo "password. This sequence is called a profile. Since the output password is not"
    echo "intended to be recorded the profile should be used to recompute it as needed."
    echo "\tIdeally, the obscured password prevents various unfortunate events from"
    echo "revealing the input password, such as:"
    echo "\t* Companies selling your data"
    echo "\t* A data breach which reveals your password or its hash"
    echo "\t* Insecure password communication protocols"
    echo ""
    echo "By way of its construction igloo changes the assumptions about your password."
    echo "Because the output cannot be generated without the input, and because the input"
    echo "is mostly useless without the profile, it becomes more important to remember the"
    echo "profile than it does the input password."
    echo "Perhaps more interesting though is that an input password can now contain special"
    echo "characters and it may be as long or short as wanted."
    echo ""
    echo "Arguments:"
    echo -e "\t-a | --hash (algorithm,)+"
    echo -e "\t\tProvide a sequence of hash algorithms to use during hashing. Duplicates"
    echo -e "\t\tare allowed and the order of hashing follows the order they are given in."
    echo -e "\t\tEach algorithm should be separated with a comma."
    echo ""
    echo -e "\t-b | --befuddle (algorithm,)+"
    echo -e "\t\tProvide a sequence of befuddle algorithms to befuddle with. Duplicates"
    echo -e "\t\tare allowed and the order of befuddling follows the order they are given in."
    echo -e "\t\tEach algorithm should be separated with a comma."
    echo ""
    echo -e "\t-h | --help [hash|befuddle]"
    echo -e "\t\tWhen no arguments are given print this help menu and exit. Otherwise"
    echo -e "\t\tprint algorithm info for befuddle."
    echo -e "\t\tThe algorithms used by hash are bash commands so refer to their man pages."
    echo -e "\t\tThe supported algorithms for hash are based on what is installed system-wide."
    echo ""
    echo -e "\t-o | --output file"
    echo -e "\t\tOutput a profile containing the hash and befuddle algorithms chosen."
    echo ""
    echo -e "\t-p | --profile file"
    echo -e "\t\tLoad hash and befuddle algorithms from a profile. Unless an absolute path is"
    echo -e "\t\tspecified this will load the profile from the $PROFILE_DIR directory."
    echo ""
    echo -e "\t-s | --silent"
    echo -e "\t\tOutput only the obscured password(s)."
    echo ""
    echo -e "\t-t | --truncate [0-9]+"
    echo -e "\t\tAfter the password is generated output only the first n characters."
    echo ""
}

PASSWORDS=()
while [[ $# -gt 0 ]]; do
    case "$1" in
            -a | --hash)
                while IFS=',' read alg; do
                    HASH_ALGS+=("$alg")
                done <<< "$2"

                shift
                shift
                ;;

            -b | --befuddle)
                # this line interprets escape sequences. This allows things like
                # \xFF (character 255) to be used in befuddle commands.
                while IFS=',' read alg; do
                    BEFUDDLE_ALGS+=(`printf "$alg"`)
                done <<< "$2"

                shift
                shift
                ;;

            -h | --help)
                case "$2" in
                    befuddle)   help_befuddle   ;;
                    *)          help_help       ;;
                esac
                exit 0
                ;;

            -o | --output)
                PROFILE_OUTPUT="$2"
                shift
                shift
                ;;

            -p | --profile)
                parse_profile "$2"
                shift
                shift
                ;;

            -s | --silent)
                SILENT=true
                shift
                ;;

            -t | --truncate)
                TRUNCATE="$2"
                shift
                shift
                ;;

            -* | --*)
                echo "Unknown option $1"
                echo "Run 'igloo --help'"
                exit 1
                ;;

            *)
                PASSWORDS+=("$1") # add a password to obscure
                shift
                ;;
    esac
done

set -- "${PASSWORDS[@]}" # restore positional parameters

main "$@"

# unset sensitive information in case script is source'd
PASSWORDS=''
PASSWORD=''
HASH_ALGS=''
BEFUDDLE_ALGS=''
alg=''
line=''

exit 0
