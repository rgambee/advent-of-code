package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
)

const (
	NUM_GENERATIONS_SHORT = 20
	NUM_GENERATIONS_LONG  = 50000000000
)

type RuleNode struct {
	plantPresentInNextGeneration bool
	children                     map[bool]*RuleNode
}

func (n *RuleNode) insertRule(plantConfig [5]bool, result bool) {
	node := n
	for _, present := range plantConfig {
		child, ok := node.children[present]
		if !ok {
			child = &RuleNode{false, make(map[bool]*RuleNode)}
			node.children[present] = child
		}
		node = child
	}
	node.plantPresentInNextGeneration = result
}

func runeToBool(r rune) bool {
	if r == '#' {
		return true
	} else if r == '.' {
		return false
	}
	log.Panic("Unrecognized character %v", r)
	return false
}

func parseRule(plantConfig string, result string) ([5]bool, bool) {
	var boolConfig [5]bool
	for i, char := range plantConfig {
		boolConfig[i] = runeToBool(char)
	}
	boolResult := runeToBool([]rune(result)[0])
	return boolConfig, boolResult
}

func decideFateOfPot(pots *map[int]bool, index int, rules *RuleNode) bool {
	node := rules
	for i := index - 2; i <= index+2; i++ {
		node = node.children[(*pots)[i]]
	}
	return node.plantPresentInNextGeneration
}

func getMinAndMaxOccupiedPots(pots *map[int]bool) (min int, max int) {
	min, max = 1000, -1000
	for index, present := range *pots {
		if present {
			if index > max {
				max = index
			}
			if index < min {
				min = index
			}
		}
	}
	return
}

func sumOccupiedPots(pots *map[int]bool) (int, int) {
	potSum := 0
	count := 0
	for i, occupied := range *pots {
		if occupied {
			potSum += i
			count++
		}
	}
	return potSum, count
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	// Read initial state
	plantPots := make(map[int]bool)
	scanner.Scan()
	initialStateRE := regexp.MustCompile("initial state: ([.#]+)")
	initialStateString := utils.ParseString(scanner.Text(), initialStateRE, 1)[0]
	for i, char := range initialStateString {
		plantPots[i] = runeToBool(char)
	}
	// Skip over blank line
	scanner.Scan()

	// Read rules
	ruleRE := regexp.MustCompile("([.#]{5}) => ([.#])")
	rules := RuleNode{false, make(map[bool]*RuleNode)}
	for scanner.Scan() {
		newRule := utils.ParseString(scanner.Text(), ruleRE, 2)
		plantConfig, result := newRule[0], newRule[1]
		rules.insertRule(parseRule(plantConfig, result))
	}

	minPlant, maxPlant := getMinAndMaxOccupiedPots(&plantPots)

	var potSumShort, potSumLong, lastPotSum, potSumBeforeLast int
	for g := 0; g < NUM_GENERATIONS_LONG; g++ {
		nextGeneration := make(map[int]bool)
		// Expand map as necessary
		plantPots[minPlant-2], plantPots[minPlant-1] = false, false
		plantPots[maxPlant+1], plantPots[maxPlant+2] = false, false
		minPlant, maxPlant = 1000, -1000
		for p := range plantPots {
			nextGeneration[p] = decideFateOfPot(&plantPots, p, &rules)
			// Update minPlant and maxPlant here instead of calling
			// getMinAndMaxOccupiedPots() to save iterating through the whole map
			if nextGeneration[p] {
				if p < minPlant {
					minPlant = p
				}
				if p > maxPlant {
					maxPlant = p
				}
			}
		}
		plantPots = nextGeneration
		potSum, _ := sumOccupiedPots(&plantPots)
		if g == NUM_GENERATIONS_SHORT-1 {
			potSumShort = potSum
		}
		if potSum-lastPotSum == lastPotSum-potSumBeforeLast {
			// Sum is changing linearly, so extrapolate
			potSumLong = potSum + (potSum-lastPotSum)*(NUM_GENERATIONS_LONG-g-1)
			break
		}
		potSumBeforeLast = lastPotSum
		lastPotSum = potSum
	}

	fmt.Println("PART 1")
	fmt.Printf("Sum of occupied pots after %v generations: %v\n",
		NUM_GENERATIONS_SHORT, potSumShort)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Printf("Sum of occupied pots after %v generations: %v\n",
		NUM_GENERATIONS_LONG, potSumLong)
}
