package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"regexp"
	"strconv"
)

func toInt(s string) int {
	num, err := strconv.Atoi(s)
	if err != nil {
		panic(err)
	}
	return num
}

func parseClaim(claim string, re *regexp.Regexp) (id, startX, startY, width, height int) {
	matches := re.FindStringSubmatch(claim)
	if matches == nil {
		log.Fatal(fmt.Sprintf(
			"No matches found for string %v and regex %v", claim, re))
	}
	// 6 matches expected: one overall match and 5 submatches
	if len(matches) != 6 {
		log.Fatal(fmt.Sprintf("Expected 6 matches but found %v"), matches)
	}

	id = toInt(matches[1])
	startX = toInt(matches[2])
	startY = toInt(matches[3])
	width = toInt(matches[4])
	height = toInt(matches[5])
	return
}

func main() {
	if len(os.Args) < 2 {
		log.Fatal("Must provide path to input file")
	}
	filename := os.Args[1]
	file, err := os.Open(filename)
	if err != nil {
		panic(err)
	}

	defer func() {
		if err := file.Close(); err != nil {
			panic(err)
		}
	}()

	reader := bufio.NewReader(file)
	scanner := bufio.NewScanner(reader)

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
		newClaim := scanner.Text()
		claimID, startX, startY, width, height := parseClaim(newClaim, claimRE)
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
