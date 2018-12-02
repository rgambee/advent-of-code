package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
)

func countCharacters(str string) map[rune]int {
	counts := make(map[rune]int)
	for _, char := range str {
		counts[char]++
	}
	return counts
}

func hasValue(theMap map[rune]int, valToLookFor int) bool {
	for _, val := range theMap {
		if val == valToLookFor {
			return true
		}
	}
	return false
}

func differByOne(str1, str2 string) bool {
	if len(str1) != len(str2) {
		return false
	}
	differenceFound := false
	for i := range str1 {
		if str1[i] != str2[i] {
			if !differenceFound {
				differenceFound = true
			} else {
				return false
			}
		}
	}
	return differenceFound
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

	doubles := 0
	triples := 0
	idArray := make([]string, 0)
	var desiredId1, desiredId2 string
	for scanner.Scan() {
		newId := scanner.Text()
		idArray = append(idArray, newId)
		counts := countCharacters(newId)
		if hasValue(counts, 2) {
			doubles++
		}
		if hasValue(counts, 3) {
			triples++
		}
		for _, id := range idArray[:len(idArray)-1] {
			if differByOne(id, newId) {
				desiredId1 = id
				desiredId2 = newId
			}
		}
	}
	fmt.Println("PART 1")
	fmt.Println("Doubles:", doubles)
	fmt.Println("Triples:", triples)
	fmt.Println("Product:", doubles*triples)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println(desiredId1)
	fmt.Println(desiredId2)
}
