#!/usr/bin/bash
set -euo pipefail
shopt -s inherit_errexit

usage="Usage: bash $0 INPUT_FILE"

number_pattern="^[0-9]+"
symbol_pattern="^[^0-9.]"
gear_pattern="^\*$"

if [[ "$#" != 1 || "${1}" == "-h" || "${1}" == "--help" ]]
then
    echo "${usage}"
    exit 0
fi
input_file="${1}"

# Build array of (x, y) coordinates to the array numbers that neighbor them
declare -A number_neighbors
y=0
while read -r line
do
    for (( x = 0; ${#line} > 0; ))
    do
        if [[ "${line}" =~ ${number_pattern} ]]
        then
            number="${BASH_REMATCH[0]}"
            for (( y_offset = -1; y_offset <= 1; ++y_offset ))
            do
                for (( x_offset = -1; x_offset <= "${#number}"; ++x_offset ))
                do
                    i=0
                    key="$(( x + x_offset )),$(( y + y_offset )),${i}"
                    while [[ -v number_neighbors["${key}"] ]]
                    do
                        (( ++i )) || true
                        key="$(( x + x_offset )),$(( y + y_offset )),${i}"
                    done
                    number_neighbors["${key}"]="${number}"
                done
            done
            line="${line:${#BASH_REMATCH[0]}}"
            (( x += ${#BASH_REMATCH[0]} )) || true
        else
            line="${line:1}"
            (( ++x )) || true
        fi
    done
    (( ++y )) || true
done < "${input_file}"

# Find all symbols and add up the numbers next to them
part_number_sum=0
gear_ratio_sum=0
y=0
while read -r line
do
    line_length="${#line}"
    for (( x = 0; x < line_length; ++x ))
    do
        if [[ "${line:${x}:1}" =~ ${symbol_pattern} ]]
        then
            i=0
            key="${x},${y},${i}"
            while [[ -v number_neighbors["${key}"] ]]
            do
                (( part_number_sum += number_neighbors["${key}"] )) || true
                (( ++i )) || true
                key="${x},${y},${i}"
            done

            if [[ "${line:${x}:1}" =~ ${gear_pattern} ]]
            then
                if [[ "${i}" == 2 ]]
                then
                    # Symbol is next to exactly two numbers
                    num0="${number_neighbors["${x},${y},0"]}"
                    num1="${number_neighbors["${x},${y},1"]}"
                    (( gear_ratio_sum += num0 * num1 ))  || true
                fi
            fi
        fi
    done
    (( ++y )) || true
done < "${input_file}"

echo "PART 1"
echo "Sum of part numbers: ${part_number_sum}"

echo "PART 2"
echo "Sum of gear ratios: ${gear_ratio_sum}"
