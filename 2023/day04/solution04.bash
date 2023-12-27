#!/usr/bin/bash
set -euo pipefail
shopt -s inherit_errexit

usage="Usage: bash $0 INPUT_FILE"

card_pattern="Card +([0-9]+):"

function count_matching_numbers() {
    numbers="${1}"
    pre_pipe="${numbers%|*}"
    post_pipe="${numbers#*|}"
    read -ra winning_numbers <<< "${pre_pipe}"
    read -ra own_numbers <<< "${post_pipe}"

    matches=0
    for onum in "${own_numbers[@]}"
    do
        for wnum in "${winning_numbers[@]}"
        do
            if [[ "${onum}" == "${wnum}" ]]
            then
               (( ++matches )) || true
                break
            fi
        done
    done
    echo "${matches}"
}

if [[ "$#" != 1 || "${1}" == "-h" || "${1}" == "--help" ]]
then
    echo "${usage}"
    exit 0
fi
input_file="${1}"

total_points=0
declare -A card_counts
while read -r line
do
    if [[ "${line}" =~ ${card_pattern} ]]
    then
        card_number="${BASH_REMATCH[1]}"
        card_contents="${line:${#BASH_REMATCH[0]}}"
        matches="$(count_matching_numbers "${card_contents}")"
        if [[ "${matches}" != 0 ]]
        then
            (( total_points += 2 ** (matches - 1) )) || true
        fi
        if [[ ! -v card_counts["${card_number}"] ]]
        then
            card_counts["${card_number}"]=0
        fi
        # Count the original card
        (( ++card_counts["${card_number}"] )) || true
        instances="${card_counts["${card_number}"]}"
        for (( i = 1; i <= matches; ++i ))
        do
            # For each instance of this card (original + copies), add copies of
            # subsequent cards
            (( following_card_number = card_number + i ))
            if [[ ! -v card_counts["${following_card_number}"] ]]
            then
                card_counts["${following_card_number}"]=0
            fi
            (( card_counts["${following_card_number}"] += instances ))
        done
    fi
done < "${input_file}"

total_cards=0
for instances in "${card_counts[@]}"
do
    (( total_cards += instances ))
done

echo "PART 1"
echo "Total points: ${total_points}"

echo "PART 2"
echo "Total number of cards: ${total_cards}"
