package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
)

func isMember(slice []int, value int) bool {
	for _, elem := range slice {
		if elem == value {
			return true
		}
	}
	return false
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)

	sum := 0
	sumAfterFirstPass := 0
	seenFrequencies := make([]int, 1)
	firstRepeatedFrequency := -1
	exit := false

	for iterations := 0; !exit; iterations++ {
		file.Seek(0, 0)
		scanner := utils.GetLineScanner(file)
		for scanner.Scan() {
			sum += utils.StringToInt(scanner.Text())

			if isMember(seenFrequencies, sum) {
				firstRepeatedFrequency = sum
				exit = true
				break
			} else {
				seenFrequencies = append(seenFrequencies, sum)
			}
		}
		if (iterations == 0) && (!exit) {
			sumAfterFirstPass = sum
		}
	}

	fmt.Println("PART 1")
	fmt.Println("Sum after first pass:", sumAfterFirstPass)
	fmt.Println("")
	fmt.Println("PART 2")
	fmt.Println("First repeated frequency:", firstRepeatedFrequency)
}
