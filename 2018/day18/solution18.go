package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
)

type CellType rune

const (
	NUM_MINUTES_SHORT          = 10
	NUM_MINUTES_LONG           = 1000000000
	PERIODICITY_CHECK          = 100
	OPEN              CellType = '.'
	TREES             CellType = '|'
	LUMBERYARD        CellType = '#'
)

type Cell struct {
	cellType CellType
}

func (c Cell) String() string {
	return string(c.cellType)
}

type CellGrid [][]Cell

func (cellGrid *CellGrid) getNeighbors(x, y int) []Cell {
	neighbors := make([]Cell, 0)
	for i := y - 1; i <= y+1; i++ {
		if i >= 0 && i < len(*cellGrid) {
			for j := x - 1; j <= x+1; j++ {
				if j >= 0 && j < len((*cellGrid)[i]) {
					if !(i == y && j == x) {
						neighbors = append(neighbors, (*cellGrid)[i][j])
					}
				}
			}
		}
	}
	if len(neighbors) > 8 {
		log.Panic("Too many neighbors", neighbors)
	}
	return neighbors
}

func (cellGrid *CellGrid) determineFateOfCell(x, y int) Cell {
	cell := (*cellGrid)[y][x]
	neighbors := cellGrid.getNeighbors(x, y)
	if cell.cellType == OPEN {
		if countCells(neighbors, TREES) >= 3 {
			return Cell{TREES}
		} else {
			return Cell{OPEN}
		}
	} else if cell.cellType == TREES {
		if countCells(neighbors, LUMBERYARD) >= 3 {
			return Cell{LUMBERYARD}
		} else {
			return Cell{TREES}
		}
	} else if cell.cellType == LUMBERYARD {
		if countCells(neighbors, TREES) >= 1 &&
			countCells(neighbors, LUMBERYARD) >= 1 {
			return Cell{LUMBERYARD}
		} else {
			return Cell{OPEN}
		}
	}
	log.Panicf("Couldn't determine fate of cell %v at (%v,%v)", cell, x, y)
	return Cell{}
}

func (cellGrid *CellGrid) printCellGrid() {
	for _, row := range *cellGrid {
		for _, cell := range row {
			fmt.Print(cell)
		}
		fmt.Println()
	}
}

func (cellGrid *CellGrid) countCells(ct CellType) int {
	count := 0
	for _, row := range *cellGrid {
		count += countCells(row, ct)
	}
	return count
}

func countCells(cells []Cell, ct CellType) int {
	count := 0
	for _, n := range cells {
		if n.cellType == ct {
			count++
		}
	}
	return count
}

func updateGrid(cellGrid *CellGrid) CellGrid {
	newGrid := make(CellGrid, len(*cellGrid))
	for y, row := range *cellGrid {
		newGrid[y] = make([]Cell, len(row))
		for x := range row {
			newGrid[y][x] = cellGrid.determineFateOfCell(x, y)
		}
	}
	return newGrid
}

func getResourceValue(cellGrid *CellGrid) int {
	return cellGrid.countCells(TREES) * cellGrid.countCells(LUMBERYARD)
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	cellGrid := make(CellGrid, 0)
	for scanner.Scan() {
		line := scanner.Text()
		row := make([]Cell, len(line))
		for i, r := range scanner.Text() {
			row[i] = Cell{CellType(r)}
		}
		cellGrid = append(cellGrid, row)
	}
	// cellGrid.printCellGrid()

	periodicity := make(map[int]int)
	lastResourceValue, predictedResourceValue := -1, -1
	periodicHitCount := 0
	for i := 0; i < NUM_MINUTES_LONG; i++ {
		if i%(NUM_MINUTES_LONG/100) == 0 {
			fmt.Printf("%v   \r", i)
		}
		if periodicHitCount >= PERIODICITY_CHECK {
			// The behavior has become periodic, so we can predict
			// the next resource value
			predictedResourceValue = periodicity[predictedResourceValue]
		} else {
			resourceValue := getResourceValue(&cellGrid)
			if predictedResourceValue == resourceValue {
				periodicHitCount++
			} else {
				periodicHitCount = 0
			}
			if i == NUM_MINUTES_SHORT {
				fmt.Println("PART 1")
				fmt.Printf("Resource value after %v minutes: %v",
					NUM_MINUTES_SHORT, resourceValue)
				fmt.Println()
			}
			if _, present := periodicity[resourceValue]; present {
				predictedResourceValue = periodicity[resourceValue]
			} else {
				predictedResourceValue = -1
			}
			periodicity[lastResourceValue] = resourceValue
			lastResourceValue = resourceValue
			cellGrid = updateGrid(&cellGrid)
		}
	}

	fmt.Println()
	fmt.Println("PART 2")
	fmt.Printf("Resouce value after %v minutes: %v\n",
		NUM_MINUTES_LONG, predictedResourceValue)
}
