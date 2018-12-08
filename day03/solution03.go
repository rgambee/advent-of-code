package main

import (
	"aoc2018/utils"
	"fmt"
	"regexp"
)

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	// Sample claim format
	//#1 @ 509,796: 18x15
	claimRE := regexp.MustCompile("#([0-9]+) @ ([0-9]+),([0-9]+): ([0-9]+)x([0-9]+)")

	fabricSize := 1000
	fabricArray := make([][][]int, fabricSize)
	for i := 0; i < fabricSize; i++ {
		fabricArray[i] = make([][]int, fabricSize)
	}

	totalMulticlaimedSquares := 0
	nonOverlappingClaim := make([]bool, 0)
	for scanner.Scan() {
		newLine := scanner.Text()
		parsedLine := utils.ParseString(newLine, claimRE, 5)
		newClaim := *utils.StringSliceToIntSlice(&parsedLine)
		claimID, startX, startY := newClaim[0], newClaim[1], newClaim[2]
		width, height := newClaim[3], newClaim[4]
		nonOverlappingClaim = append(nonOverlappingClaim, true)
		for x := startX; x < startX+width; x++ {
			for y := startY; y < startY+height; y++ {
				fabricArray[x][y] = append(fabricArray[x][y], claimID)
				if len(fabricArray[x][y]) == 2 {
					totalMulticlaimedSquares++
				}
				if len(fabricArray[x][y]) > 1 {
					for _, id := range fabricArray[x][y] {
						nonOverlappingClaim[id-1] = false
					}
				}
			}
		}
	}

	fmt.Println("PART 1")
	fmt.Println("Overlapping square inches:", totalMulticlaimedSquares)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Non-overlapping claim(s):")
	for claimID, isValid := range nonOverlappingClaim {
		if isValid {
			fmt.Println(claimID + 1)
		}
	}
}
