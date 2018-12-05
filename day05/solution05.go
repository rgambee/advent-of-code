package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"log"
	"math"
	"os"
)

func copySlice(s *[]byte) *[]byte {
	newSlice := make([]byte, len(*s))
	copy(newSlice, *s)
	return &newSlice
}

func willAnnihilate(a, b byte) bool {
	return math.Abs(float64(a)-float64(b)) == 32.0
}

func annihilate(polymer *[]byte, i, j int) {
	*polymer = append((*polymer)[:i], (*polymer)[j:]...)
}

func removeUnit(polymer *[]byte, unitToRemove byte) {
	for i := 0; i < len(*polymer); {
		if (*polymer)[i] == unitToRemove || willAnnihilate((*polymer)[i], unitToRemove) {
			annihilate(polymer, i, i+1)
		} else {
			i++
		}
	}
}

func reactPolymer(polymer *[]byte) {
	keepLooking := true
	for keepLooking {
		keepLooking = false
		for i := 0; i < len(*polymer)-1; {
			if willAnnihilate((*polymer)[i], (*polymer)[i+1]) {
				annihilate(polymer, i, i+2)
				keepLooking = true
			} else {
				i++
			}
		}
	}
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
	originalPolymer, err := ioutil.ReadAll(reader)
	if err != nil {
		panic(err)
	}

	reactedPolymer := copySlice(&originalPolymer)
	reactPolymer(reactedPolymer)
	shortestLength := len(originalPolymer)
	for unit := byte('A'); unit <= byte('Z'); unit++ {
		cleanedPolymer := copySlice(&originalPolymer)
		removeUnit(cleanedPolymer, unit)
		reactPolymer(cleanedPolymer)
		fmt.Println("Reacted length after removing", string(unit),
			len(*cleanedPolymer))
		if len(*cleanedPolymer) < shortestLength {
			shortestLength = len(*cleanedPolymer)
		}
	}

	fmt.Println()
	fmt.Println("PART 1")
	fmt.Println("Reacted length:", len(*reactedPolymer))
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Shortest reacted length:", shortestLength)
}
