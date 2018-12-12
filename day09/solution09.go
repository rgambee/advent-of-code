package main

import (
	"container/ring"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"regexp"
)

func newMarble(value int) *ring.Ring {
	marble := ring.New(1)
	marble.Value = value
	return marble
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	re := regexp.MustCompile("([0-9]+) players; last marble is worth ([0-9]+) points")

	for part := 1; scanner.Scan(); part++ {
		gameFormat := utils.ParseString(scanner.Text(), re, 2)
		numPlayers := utils.StringToInt(gameFormat[0])
		lastMarbleValue := utils.StringToInt(gameFormat[1])

		scores := make([]int, numPlayers)

		currMarble := newMarble(0)

		for newMarbleValue := 1; newMarbleValue <= lastMarbleValue; newMarbleValue++ {
			if newMarbleValue%23 != 0 {
				newMarble := newMarble(newMarbleValue)
				currMarble.Next().Link(newMarble)
				currMarble = newMarble
			} else {
				activePlayer := (newMarbleValue - 1) % numPlayers
				scores[activePlayer] += newMarbleValue
				marbleToRemove := currMarble.Move(-7)
				currMarble = marbleToRemove.Next()
				scores[activePlayer] += marbleToRemove.Value.(int)
				marbleToRemove.Prev().Unlink(1)
			}
		}
		_, highestScore := utils.FindSliceMax(&scores)
		fmt.Println("PART ", part)
		fmt.Println(numPlayers, "players and last marble value of", lastMarbleValue)
		fmt.Println("Highest score:", highestScore)
		fmt.Println()
	}
}
