package main

import (
	"fmt"
)

const NUM_RECIPES = 360781
const NUM_SCORES_AFTER_TARGET = 10

func splitDigits(n int) []int {
	// reverse
	digits := make([]int, 0)
	for n > 0 {
		digits = append(digits, n%10)
		n /= 10
	}
	if len(digits) == 0 {
		digits = make([]int, 1)
	}
	// Reverse digits so most significant is first
	for i := 0; i < len(digits)/2; i++ {
		j := len(digits) - 1 - i
		digits[i], digits[j] = digits[j], digits[i]
	}
	return digits
}

func generateNewRecipe(recipes *[]int, elf1, elf2 int) (int, int) {
	elf1Score, elf2Score := (*recipes)[elf1], (*recipes)[elf2]
	newScores := splitDigits(elf1Score + elf2Score)
	*recipes = append(*recipes, newScores...)
	elf1 = (elf1 + elf1Score + 1) % len(*recipes)
	elf2 = (elf2 + elf2Score + 1) % len(*recipes)
	return elf1, elf2
}

func main() {
	pattern := splitDigits(NUM_RECIPES)
	patternIndex := 0
	recipeIndex := 0

	recipes := make([]int, 2)
	recipes[0] = 3
	recipes[1] = 7
	elf1, elf2 := 0, 1

	for i := 0; i < NUM_RECIPES+NUM_SCORES_AFTER_TARGET || patternIndex < len(pattern); i++ {
		elf1, elf2 = generateNewRecipe(&recipes, elf1, elf2)
		for ; recipeIndex < len(recipes); recipeIndex++ {
			if recipes[recipeIndex] == pattern[patternIndex] {
				patternIndex++
				if patternIndex >= len(pattern) {
					break
				}
			} else {
				patternIndex = 0
			}
		}
	}
	fmt.Println("PART 1")
	fmt.Print("Scores after target number of recipes: ")
	for i := NUM_RECIPES; i < NUM_RECIPES+NUM_SCORES_AFTER_TARGET; i++ {
		fmt.Printf("%v", recipes[i])
	}
	fmt.Println()
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Number of recipes until pattern is reached:", recipeIndex-len(pattern)+1)
}
