#!/usr/bin/bash
set -euo pipefail
shopt -s inherit_errexit

usage="Usage: bash $0 INPUT_FILE"

digit_pattern='^[0-9]'
word_patterns=(
    '$^'    # Unmatchable placeholder since zero isn't included
    '^one'
    '^two'
    '^three'
    '^four'
    '^five'
    '^six'
    '^seven'
    '^eight'
    '^nine'
)
# Longest number in the array above
max_word_length=5

function parse_calibration() {
    local input_file="${1}"
    local include_words="${2}"
    local sum=0

    while read -r line
    do
        local first=
        local last=
        for (( i = 0; i < ${#line}; ++i ))
        do
            # Look for digits: 0, 1, 2, ...
            local char="${line:${i}:1}"
            if [[ "${char}" =~ ${digit_pattern} ]]
            then
                if [[ -z "${first}" ]]
                then
                    first="${char}"
                fi
                last="${char}"
                continue
            fi

            if [[ "${include_words}" == true ]]
            then
                # Look for number words: one, two, three, ...
                for j in "${!word_patterns[@]}"
                do
                    if [[ "${line:${i}:${max_word_length}}" =~ ${word_patterns[${j}]} ]]
                    then
                        if [[ -z "${first}" ]]
                        then
                            first="${j}"
                        fi
                        last="${j}"
                        break
                    fi
                done
            fi

        done
        if [[ -z "${first}" || -z "${last}" ]]
        then
            echo "Failed to find numbers in line ${line}" >&2
            return 1
        fi
        (( sum += 10 * first + last )) || true
    done < "${input_file}"
    echo "${sum}"
}

if [[ "$#" != 1 || "${1}" == "-h" || "${1}" == "--help" ]]
then
    echo "${usage}"
    exit 0
fi
input_file="${1}"

echo "PART 1"
echo -n "Sum of calibration values: "
parse_calibration "${input_file}" false

echo "PART 2"
echo -n "Sum of calibration values: "
parse_calibration "${input_file}" true
