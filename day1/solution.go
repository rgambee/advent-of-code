package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
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

	sum := 0
	seenFrequencies := make([]int, 1)
	exit := false

	for iterations := 0; !exit; iterations++ {
		file.Seek(0, 0)
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			num, err := strconv.Atoi(scanner.Text())
			if err != nil {
				panic(err)
			}
			sum += num

			if isMember(seenFrequencies, sum) {
				fmt.Println(sum)
				exit = true
				break
			} else {
				seenFrequencies = append(seenFrequencies, sum)
			}
		}
		if (iterations == 0) && (!exit) {
			fmt.Println("Sum after first pass:", sum)
		}
	}
}
