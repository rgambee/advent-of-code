#!/usr/bin/bash
set -euo pipefail
shopt -s inherit_errexit

usage="Usage: bash $0 INPUT_FILE"

id_regex="^Game ([0-9]+): "

function max_cube_count() {
    local game="${1}"
    local color="${2}"
    local max=0
    local pattern="^([0-9]+) ${color}"
    while [[ -n "${game}" ]]
    do
        if [[ "${game}" =~ ${pattern} ]]
        then
            cubes="${BASH_REMATCH[1]}"
            if [[ "${cubes}" -gt "${max}" ]]
            then
                max="${cubes}"
            fi
            game="${game:${#BASH_REMATCH[0]}}"
        else
            game="${game:1}"
        fi

    done
    echo "${max}"
}

function game_id_if_possible() {
    local game="${1}"
    shift

    if [[ ${game} =~ ${id_regex} ]]
    then
        local game_id="${BASH_REMATCH[1]}"
        game="${game:${#BASH_REMATCH[0]}}"

        while [[ "${#}" -gt 0 ]]
        do
            if [[ "${#}" == 1 ]]
            then
                echo "Arguments must be pairs of colors and max allowed cubes" 1>&2
                exit 1
            fi

            color="${1}"
            max_allowed="${2}"
            shift 2

            local max
            max="$(max_cube_count "${game}" "${color}")"
            if [[ "${max}" -gt "${max_allowed}" ]]
            then
                # Game is impossible. Echo 0 so its ID doesn't get counted.
                echo 0
                exit 0
            fi
        done

        # Game is possible. Echo its ID so it gets counted.
        echo "${game_id}"
    else
        echo "No match for game ID" 1>&2
        exit 2
    fi
}

function add_possible_games() {
    local input_file="${1}"
    shift
    local colors_and_counts=( "${@}" )
    local sum=0

    while read -r line
    do
        game_id="$(game_id_if_possible "${line}" "${colors_and_counts[@]}")"
        (( sum += game_id )) || true
    done < "${input_file}"
    echo "${sum}"
}

function compute_min_set_power() {
    local game="${1}"
    shift
    local power=1

    while [[ "${#}" -gt 0 ]]
    do
        color="${1}"
        shift

        local max
        max="$(max_cube_count "${game}" "${color}")"
        (( power *= max )) || true
    done
    echo "${power}"
}

function add_min_powers() {
    local input_file="${1}"
    shift
    local colors=( "${@}" )
    local sum=0

    while read -r line
    do
        set_power="$(compute_min_set_power "${line}" "${colors[@]}")"
        (( sum += set_power )) || true
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
echo -n "Sum of valid game IDs: "
add_possible_games "${input_file}" red 12 green 13 blue 14

echo "PART 2"
echo -n "Sum of powers of minimum sets: "
add_min_powers "${input_file}" red green blue
