package utils

import (
	"bufio"
	"log"
	"os"
	"regexp"
	"strconv"
)

func GetFilename() *string {
	if len(os.Args) < 2 {
		log.Panic("Must provide path to input file")
	}
	filename := os.Args[1]
	return &filename
}

func OpenFile(filename *string) *os.File {
	file, err := os.Open(*filename)
	if err != nil {
		panic(err)
	}
	return file
}

func CloseFile(file *os.File) {
	if err := file.Close(); err != nil {
		panic(err)
	}
}

func GetLineScanner(file *os.File) *bufio.Scanner {
	return GetScanner(file, bufio.ScanLines)
}

func GetWordScanner(file *os.File) *bufio.Scanner {
	return GetScanner(file, bufio.ScanWords)
}

func GetScanner(file *os.File, split bufio.SplitFunc) *bufio.Scanner {
	reader := bufio.NewReader(file)
	scanner := bufio.NewScanner(reader)
	scanner.Split(split)
	return scanner
}

func ParseString(s string, re *regexp.Regexp, expectedMatches int) []string {
	matches := re.FindStringSubmatch(s)
	if matches == nil {
		log.Fatal("No matches found for ", s)
	}
	// First element of matches is the match for the entire regexp,
	// which we don't care about
	if len(matches) != expectedMatches+1 {
		log.Fatal("Expected %v matches but found %v", expectedMatches, len(matches)-1)
	}
	return matches[1:]
}

func StringToInt(s string) int {
	num, err := strconv.Atoi(s)
	if err != nil {
		panic(err)
	}
	return num
}

func StringSliceToIntSlice(stringSlice *[]string) *[]int {
	intSlice := make([]int, len(*stringSlice))
	for i, s := range *stringSlice {
		intSlice[i] = StringToInt(s)
	}
	return &intSlice
}

func SumSlice(slice *[]int) int {
	total := 0
	for _, n := range *slice {
		total += n
	}
	return total
}

func FindSliceMax(slice *[]int) (index, max int) {
	index = -1
	max = -1
	for i, elem := range *slice {
		if elem > max {
			max = elem
			index = i
		}
	}
	return
}
