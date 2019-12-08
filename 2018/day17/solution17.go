package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"image"
	"log"
	"regexp"
)

type BoundingBox image.Rectangle

func (bbox *BoundingBox) update(x, y int) {
	uninitialized := (bbox.Min.X == 0 && bbox.Max.X == 0 && bbox.Min.Y == 0 && bbox.Max.Y == 0)
	if uninitialized || x < bbox.Min.X {
		bbox.Min.X = x
	}
	if uninitialized || x > bbox.Max.X {
		bbox.Max.X = x
	}
	if uninitialized || y < bbox.Min.Y {
		bbox.Min.Y = y
	}
	if uninitialized || y > bbox.Max.Y {
		bbox.Max.Y = y
	}
}

type CellType rune

const (
	DRY_SAND      CellType = '.'
	WET_SAND      CellType = '|'
	CLAY          CellType = '#'
	SETTLED_WATER CellType = '~'
)

var DOWN image.Point = image.Point{0, 1}
var LEFT image.Point = image.Point{-1, 0}
var RIGHT image.Point = image.Point{1, 0}

type Cell struct {
	cellType CellType
}

func (c Cell) isPassable() bool {
	return c.cellType == DRY_SAND || c.cellType == WET_SAND
}

func (c Cell) isWet() bool {
	return c.cellType == WET_SAND || c.cellType == SETTLED_WATER
}

type CellGrid map[int]map[int]Cell

// TODO method getCell(image.Point) Cell

func (cellGrid *CellGrid) spreadWater(from, to image.Point, debug bool) (bool, bool) {
	if debug {
		fmt.Printf("Working on cell %v\n", to)
	}
	if debug {
		printCellGrid(cellGrid, BoundingBox{
			image.Point{to.X - 50, to.Y - 10},
			image.Point{to.X + 50, to.Y + 10}})
	}
	didAnythingChange := false
	ableToSpreadDown := false
	if debug {
		fmt.Printf("Spreading from %v to %v\n", from, to)
	}
	if _, present := (*cellGrid)[to.Y]; !present {
		if debug {
			wetCells, settedCells := countWetCells(cellGrid)
			fmt.Printf("Wet cells: %v, settled cells: %v\n", wetCells, settedCells)
			// log.Panicf("Y coodinate of %v is not in cellGrid\n", to)
		}
		return false, false
	}
	if (*cellGrid)[to.Y][to.X].cellType == DRY_SAND {
		if debug {
			fmt.Printf("Setting cell at %v to %v\n", to, string(WET_SAND))
		}
		(*cellGrid)[to.Y][to.X] = Cell{WET_SAND}
		didAnythingChange = true
	}
	downCell, present := (*cellGrid)[to.Y+1][to.X]
	if debug {
		fmt.Printf("Cell below is %v at %v\n", string(downCell.cellType), to.Add(DOWN))
	}
	if !present || downCell.isPassable() {
		// Spread downwards since we can
		recusiveDidAnythingChange, _ := cellGrid.spreadWater(
			to, to.Add(image.Point{0, 1}), debug)
		didAnythingChange = didAnythingChange || recusiveDidAnythingChange
		ableToSpreadDown = true
	} else {
		// Can't spread down, so spread sideways
		direction := to.Add(image.Point{-from.X, -from.Y}) // direction = to - from
		recursiveDAC1, recursiveDAC2 := false, false
		recursiveATSD1, recursiveATSD2 := false, false
		if direction == DOWN {
			// Water is entering from above,
			// so try to spread to both sides
			if debug {
				fmt.Println("Can't spread down, so trying to spread left and right")
			}
			if (*cellGrid)[to.Y][to.X-1].isPassable() {
				recursiveDAC1, recursiveATSD1 = cellGrid.spreadWater(
					to, to.Add(LEFT), debug)
			}
			if (*cellGrid)[to.Y][to.X+1].isPassable() {
				recursiveDAC2, recursiveATSD2 = cellGrid.spreadWater(
					to, to.Add(RIGHT), debug)
			}
			ableToSpreadDown = ableToSpreadDown || recursiveATSD1 || recursiveATSD2
			if !ableToSpreadDown {
				// Mark as settled water
				if (*cellGrid)[to.Y][to.X].cellType != WET_SAND {
					log.Panicf("Expecting cell at %v to be wet sand but is %v\n",
						to, string((*cellGrid)[to.Y][to.X].cellType))
				}
				if debug {
					fmt.Printf("Setting cell at %v to %v\n", to, string(SETTLED_WATER))
				}
				(*cellGrid)[to.Y][to.X] = Cell{SETTLED_WATER}
				didAnythingChange = true
				// Set cells to either side as settled water too
				for pt := to.Add(LEFT); ; pt = pt.Add(LEFT) {
					if !(*cellGrid)[pt.Y][pt.X].isPassable() {
						break
					}
					if (*cellGrid)[pt.Y][pt.X].cellType != WET_SAND {
						log.Panicf("Expecting cell at %v to be wet sand but is %v\n",
							to, string((*cellGrid)[to.Y][to.X].cellType))
					}
					(*cellGrid)[pt.Y][pt.X] = Cell{SETTLED_WATER}
				}
				for pt := to.Add(RIGHT); ; pt = pt.Add(RIGHT) {
					if !(*cellGrid)[pt.Y][pt.X].isPassable() {
						break
					}
					if (*cellGrid)[pt.Y][pt.X].cellType != WET_SAND {
						log.Panicf("Expecting cell at %v to be wet sand but is %v\n",
							to, string((*cellGrid)[to.Y][to.X].cellType))
					}
					(*cellGrid)[pt.Y][pt.X] = Cell{SETTLED_WATER}
				}
			}
		} else if direction == LEFT || direction == RIGHT {
			// Water is entering from the side,
			// so keep spreading in that direction
			if debug {
				fmt.Println("Trying to continue moving sideways")
			}
			if (*cellGrid)[to.Add(direction).Y][to.Add(direction).X].isPassable() {
				recursiveDAC1, recursiveATSD1 = cellGrid.spreadWater(
					to, to.Add(direction), debug)
			}
		} else {
			log.Panic("Water can't flow up")
		}
		didAnythingChange = didAnythingChange || recursiveDAC1 || recursiveDAC2
		ableToSpreadDown = ableToSpreadDown || recursiveATSD1 || recursiveATSD2
	}
	return didAnythingChange, ableToSpreadDown
}

func printCellGrid(cellGrid *CellGrid, bbox BoundingBox) {
	for y := bbox.Min.Y; y <= bbox.Max.Y; y++ {
		for x := bbox.Min.X; x <= bbox.Max.X; x++ {
			if y == bbox.Min.Y {
				fmt.Printf("%v", x%10)
			} else {
				fmt.Printf(string((*cellGrid)[y][x].cellType))
			}
		}
		fmt.Printf(" %v\n", y)
	}
}

func countWetCells(cellGrid *CellGrid) (int, int) {
	totalWetCells := 0
	settledWaterCells := 0
	for _, row := range *cellGrid {
		for _, cell := range row {
			if cell.isWet() {
				totalWetCells++
				if cell.cellType == SETTLED_WATER {
					settledWaterCells++
				}
			}
		}
	}
	return totalWetCells, settledWaterCells
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	clayRE := regexp.MustCompile("([xy])=(\\d+), ([xy])=(\\d+)..(\\d+)")
	clayVeins := make([]image.Rectangle, 0)
	bbox := BoundingBox{}

	for scanner.Scan() {
		segInfo, err := utils.ParseString(scanner.Text(), clayRE, 5)
		if err != nil {
			panic(err)
		}
		fixedCoord := utils.StringToInt(segInfo[1])
		rangeCoord0, rangeCoord1 := utils.StringToInt(segInfo[3]), utils.StringToInt(segInfo[4])
		if segInfo[0] == "x" {
			// Vertical vein of clay
			clayVeins = append(clayVeins, image.Rect(
				fixedCoord, rangeCoord0, fixedCoord, rangeCoord1))
			bbox.update(fixedCoord, rangeCoord0)
			bbox.update(fixedCoord, rangeCoord1)
		} else if segInfo[0] == "y" {
			// Horizontal vein of clay
			clayVeins = append(clayVeins, image.Rect(
				rangeCoord0, fixedCoord, rangeCoord1, fixedCoord))
			bbox.update(rangeCoord0, fixedCoord)
			bbox.update(rangeCoord1, fixedCoord)
		} else {
			log.Panic("Unexpected input:", segInfo)
		}
	}
	// Expand bounding box by 1 in +/- x to allow water to flow down sides
	bbox.Min.X -= 1
	bbox.Max.X += 1
	// fmt.Println("Bounding box is", bbox)

	// Initialize grid to be all dry sand
	cellGrid := make(CellGrid)
	for y := bbox.Min.Y; y <= bbox.Max.Y; y++ {
		cellGrid[y] = make(map[int]Cell)
		for x := bbox.Min.X; x <= bbox.Max.X; x++ {
			cellGrid[y][x] = Cell{DRY_SAND}
		}
	}
	// Fill in clay veins
	for _, vein := range clayVeins {
		for y := vein.Min.Y; y <= vein.Max.Y; y++ {
			for x := vein.Min.X; x <= vein.Max.X; x++ {
				cellGrid[y][x] = Cell{CLAY}
			}
		}
	}
	// printCellGrid(&cellGrid, bbox)
	keepGoing := true
	for i := 0; keepGoing; i++ {
		// printCellGrid(&cellGrid, bbox)
		fmt.Println(i)
		debug := false
		keepGoing, _ = cellGrid.spreadWater(
			image.Point{500, bbox.Min.Y - 1}, image.Point{500, bbox.Min.Y},
			debug)
	}

	// printCellGrid(&cellGrid, BoundingBox{image.Point{bbox.Min.X, 500}, image.Point{bbox.Max.X, 1000}})
	// printCellGrid(&cellGrid, bbox)
	numberOfWetCells, numberOfSettledWaterCells := countWetCells(&cellGrid)

	fmt.Println()
	fmt.Println("Total number of wet cells is", numberOfWetCells)
	fmt.Println()
	fmt.Println("Number of cells of settled water is", numberOfSettledWaterCells)
}
