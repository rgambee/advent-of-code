package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
)

const (
	GRID_SIZE   = 300
	GRID_SERIAL = 1718
)

func main() {
	grid := *(utils.MakeSquareIntSlice(GRID_SIZE))
	// summedAreaTable contians sum of all cells above and to the left
	summedAreaTable := *(utils.MakeSquareIntSlice(GRID_SIZE + 1))

	for x := range grid {
		for y := range grid[x] {
			// In instructions, x and y coordinates are indexed from 1
			x_indexFrom1, y_indexFrom1 := x+1, y+1
			rackID := x_indexFrom1 + 10
			powerLevel := rackID * y_indexFrom1
			powerLevel += GRID_SERIAL
			powerLevel *= rackID
			powerLevel = (powerLevel / 100) % 10
			powerLevel -= 5
			grid[x][y] = powerLevel

			// summedAreaTable is offset by 1 in x and y for simplicity
			summedArea10 := summedAreaTable[x][y+1]
			summedArea01 := summedAreaTable[x+1][y]
			summedArea11 := summedAreaTable[x][y]
			summedAreaTable[x+1][y+1] = powerLevel + summedArea10 + summedArea01 - summedArea11
		}
	}

	highest3x3Sum := -1000
	highest3x3SumX, highest3x3SumY := -1, -1
	highestOverallSum := -1000
	highestOverallSize := -1
	highestOverallSumX, highestOverallSumY := -1, -1

	for squareSize := 1; squareSize <= GRID_SIZE; squareSize++ {
		for x := range grid[:GRID_SIZE-squareSize+1] {
			for y := range grid[0][:GRID_SIZE-squareSize+1] {
				var squareSum int
				if squareSize == 1 {
					squareSum = grid[x][y]
				} else {
					squareSum = (summedAreaTable[x][y] -
						summedAreaTable[x+squareSize][y] -
						summedAreaTable[x][y+squareSize] +
						summedAreaTable[x+squareSize][y+squareSize])
				}
				if squareSum > highestOverallSum {
					highestOverallSum = squareSum
					highestOverallSize = squareSize
					highestOverallSumX, highestOverallSumY = x, y
				}
				if squareSize == 3 {
					if squareSum > highest3x3Sum {
						highest3x3Sum = squareSum
						highest3x3SumX, highest3x3SumY = x, y
					}
				}
			}
		}
	}

	fmt.Println("PART 1")
	fmt.Printf("Coordinates (indexing from 1) of 3x3 square with highest sum: %v,%v\n",
		highest3x3SumX+1, highest3x3SumY+1)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Printf("Square with highest sum has coordinates (indexing from 1) and size of: %v,%v,%v\n",
		highestOverallSumX+1, highestOverallSumY+1, highestOverallSize)
}
