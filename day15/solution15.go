package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
)

type Symbol rune

const (
	FLOOR_SYMBOL  Symbol = '.'
	WALL_SYMBOL   Symbol = '#'
	ELF_SYMBOL    Symbol = 'E'
	GOBLIN_SYMBOL Symbol = 'G'

	STARTING_HITPOINTS = 200
	ATTACK_POWER       = 3
	MAX_DIST           = 1<<31 - 1
)

type Cell struct {
	cellType Symbol
	occupant *Humanoid
}

func newCell(sym Symbol, attackPower int) *Cell {
	if !(sym == FLOOR_SYMBOL || sym == WALL_SYMBOL || sym == ELF_SYMBOL || sym == GOBLIN_SYMBOL) {
		log.Panicf("Unrecognized symbol '%v'", string(sym))
	}
	cell := Cell{}
	if sym == WALL_SYMBOL {
		cell.cellType = sym
	} else {
		if sym == ELF_SYMBOL {
			newElf(&cell, attackPower)
		} else if sym == GOBLIN_SYMBOL {
			newGoblin(&cell, attackPower)
		}
		cell.cellType = FLOOR_SYMBOL
	}
	return &cell
}

func (cell *Cell) copyCell() *Cell {
	cellCopy := *cell
	if cell.occupant != nil {
		occupantCopy := *cell.occupant
		cellCopy.occupant = &occupantCopy
		cellCopy.occupant.location = &cellCopy
	}
	return &cellCopy
}

func (cell *Cell) isPassable() bool {
	return cell.cellType == FLOOR_SYMBOL && cell.occupant == nil
}

func (cell *Cell) getAppearance() string {
	if cell.occupant != nil {
		return string(cell.occupant.race)
	}
	return string(cell.cellType)
}

type Humanoid struct {
	race            Symbol
	hitpoints       int
	attackPower     int
	location        *Cell
	nextRoundToMove int
}

func newHumanoid(location *Cell, race Symbol, attackPower int) *Humanoid {
	hum := Humanoid{
		race: race, hitpoints: STARTING_HITPOINTS,
		attackPower: attackPower, location: location}
	location.occupant = &hum
	return &hum
}

func newElf(location *Cell, attackPower int) *Humanoid {
	return newHumanoid(location, ELF_SYMBOL, attackPower)
}

func newGoblin(location *Cell, attackPower int) *Humanoid {
	return newHumanoid(location, GOBLIN_SYMBOL, attackPower)
}

func (attacker *Humanoid) attack(target *Humanoid) {
	if attacker.race == target.race {
		log.Panicf("Attacking own kind %v -> %v", attacker, target)
	}
	if target.hitpoints <= 0 {
		log.Panicf("Attacking dead target %v -> %v", attacker, target)
	}
	target.hitpoints -= attacker.attackPower
	if target.hitpoints <= 0 {
		target.location.occupant = nil
	}
}

func (hum *Humanoid) moveTo(cell *Cell) {
	if !cell.isPassable() {
		log.Panicf("Attempting to move %v to impassible cell %v",
			hum, cell)
	}
	hum.location.occupant = nil
	hum.location = cell
	hum.location.occupant = hum
}

func printCellGrid(cellGrid *[][]*Cell) {
	for _, row := range *cellGrid {
		humanoidHitpointsInRow := make([]int, 0)
		for _, cell := range row {
			fmt.Print(cell.getAppearance())
			if cell.occupant != nil {
				humanoidHitpointsInRow = append(
					humanoidHitpointsInRow, cell.occupant.hitpoints)
			}
		}
		if len(humanoidHitpointsInRow) > 0 {
			fmt.Printf("%v", humanoidHitpointsInRow)
		}
		fmt.Print("\n")
	}
}

func copyCellGrid(cellGrid *[][]*Cell) [][]*Cell {
	cellGridCopy := make([][]*Cell, len(*cellGrid))
	for y := 0; y < len(*cellGrid); y++ {
		row := make([]*Cell, len((*cellGrid)[y]))
		for x := 0; x < len((*cellGrid)[y]); x++ {
			row[x] = (*cellGrid)[y][x].copyCell()
		}
		cellGridCopy[y] = row
	}
	return cellGridCopy
}

func setElfAttackPower(cellGrid *[][]*Cell, elfAttackPower int) {
	for y := 0; y < len(*cellGrid); y++ {
		for x := 0; x < len((*cellGrid)[y]); x++ {
			cell := (*cellGrid)[y][x]
			if cell.occupant != nil && cell.occupant.race == ELF_SYMBOL {
				cell.occupant.attackPower = elfAttackPower
			}
		}
	}
}

func getReadingOrderNeighbors(x, y int) [4][2]int {
	// Returns the coordinates of the four orthogonal neighbors
	// in reading order
	neighbors := [4][2]int{
		[2]int{x, y - 1},
		[2]int{x - 1, y},
		[2]int{x + 1, y},
		[2]int{x, y + 1}}
	return neighbors
}

// TODO: Make pathfinding more efficient
//			Don't compute distance twice (finding nearest opponent and again in getting route)
//				Reuse distGrid somehow

func findShortestDistance(cellGrid *[][]*Cell, xStart, yStart, xDest, yDest int) int {
	// Initialize distGrid with -1
	distGrid := make([][]int, len(*cellGrid))
	for y := range distGrid {
		distGrid[y] = make([]int, len((*cellGrid)[y]))
		for x := range distGrid[y] {
			distGrid[y][x] = -1
		}
	}
	// Set distance at (yDest, xDest) to 0
	distGrid[yDest][xDest] = 0
	cellsToCheck := make([][2]int, 1)
	cellsToCheck[0] = [2]int{xDest, yDest}
	// Perform breadth-first fill
	for radius := 1; len(cellsToCheck) > 0; radius++ {
		// Fill in all cells at this radius
		cellsToCheckNext := make([][2]int, 0)
		for _, coords := range cellsToCheck {
			neighboringCoords := getReadingOrderNeighbors(coords[0], coords[1])
			for _, neighCoords := range neighboringCoords {
				xNeigh, yNeigh := neighCoords[0], neighCoords[1]
				if yNeigh < 0 || yNeigh >= len(*cellGrid) ||
					xNeigh < 0 || xNeigh >= len((*cellGrid)[yNeigh]) {
					continue
				}
				if xNeigh == xStart && yNeigh == yStart {
					return radius
				}
				if distGrid[yNeigh][xNeigh] >= 0 {
					continue
				}
				if !(*cellGrid)[yNeigh][xNeigh].isPassable() {
					distGrid[yNeigh][xNeigh] = MAX_DIST
					continue
				}
				distGrid[yNeigh][xNeigh] = radius
				cellsToCheckNext = append(cellsToCheckNext, neighCoords)
			}
		}
		cellsToCheck = cellsToCheckNext
	}
	return MAX_DIST
}

func findShortestRoute(cellGrid *[][]*Cell, xStart, yStart, xDest, yDest int, debug bool) (int, int) {
	// Given a starting point and a destination, return the coordinates
	// neighboring the starting point that best move toward the destination
	neighboringCoords := getReadingOrderNeighbors(xStart, yStart)
	neighborDists := make([]int, len(neighboringCoords))
	for i, coords := range neighboringCoords {
		xNeigh, yNeigh := coords[0], coords[1]
		if xNeigh == xDest && yNeigh == yDest {
			neighborDists[i] = 0
		} else if (*cellGrid)[yNeigh][xNeigh].isPassable() {
			neighborDists[i] = findShortestDistance(
				cellGrid, xNeigh, yNeigh, xDest, yDest)
		} else {
			neighborDists[i] = MAX_DIST
		}
	}
	ind, minDist := utils.FindSliceMin(&neighborDists)
	if minDist == MAX_DIST {
		// No valid route to destination
		return xStart, yStart
	}
	return neighboringCoords[ind][0], neighboringCoords[ind][1]
}

func findNearestOpponent(cellGrid *[][]*Cell, xStart, yStart int, debug bool) (int, int) {
	startingCell := (*cellGrid)[yStart][xStart]
	hum := startingCell.occupant
	if hum == nil {
		log.Panicf("Can't find opponent for unoccupied cell %v", startingCell)
		return -1, -1
	}
	opponentsCoords := make([][2]int, 0)
	opponentDists := make([]int, 0)
	for y := 0; y < len(*cellGrid); y++ {
		for x := 0; x < len((*cellGrid)[y]); x++ {
			cell := (*cellGrid)[y][x]
			if cell.occupant != nil && cell.occupant.race != hum.race {
				dist := findShortestDistance(
					cellGrid, xStart, yStart, x, y)
				if debug {
					fmt.Printf("Opponent at (%v,%v) is %v away\n", x, y, dist)
				}
				if dist < MAX_DIST {
					opponentsCoords = append(opponentsCoords, [2]int{x, y})
					opponentDists = append(opponentDists, dist)
				}
			}
		}
	}
	if len(opponentsCoords) == 0 {
		// No opponents found
		return -1, -1
	}
	ind, _ := utils.FindSliceMin(&opponentDists)
	return opponentsCoords[ind][0], opponentsCoords[ind][1]
}

func findWeakestNeighboringOpponent(cellGrid *[][]*Cell, x, y int) *Humanoid {
	hum := (*cellGrid)[y][x].occupant
	if hum == nil {
		log.Panicf(
			"Can't find opponent of non-existent humanoid at (%v,%v)", x, y)
	}
	opponents := make([]*Humanoid, 0)
	opponentHitpoints := make([]int, 0)
	neighboringCoords := getReadingOrderNeighbors(x, y)
	for _, coords := range neighboringCoords {
		xNeigh, yNeigh := coords[0], coords[1]
		if yNeigh < 0 || yNeigh >= len(*cellGrid) || xNeigh < 0 || xNeigh >= len((*cellGrid)[y]) {
			continue
		}
		oppNeigh := (*cellGrid)[yNeigh][xNeigh].occupant
		if oppNeigh != nil && oppNeigh.race != (*cellGrid)[y][x].occupant.race {
			opponents = append(opponents, oppNeigh)
			opponentHitpoints = append(opponentHitpoints, oppNeigh.hitpoints)
		}
	}
	if len(opponents) == 0 {
		// No neighboring opponents found
		return nil
	}
	ind, _ := utils.FindSliceMin(&opponentHitpoints)
	return opponents[ind]
}

func simulateBattle(cellGrid *[][]*Cell, abortOnElfDeath bool) (int, bool) {
	didAnyElvesDie := false
	stop := false
	round := 0
	for ; !stop; round++ {
		numHumanoids := make(map[Symbol]int)
		for y := 0; y < len(*cellGrid); y++ {
			for x := 0; x < len((*cellGrid)[y]); x++ {
				cell := (*cellGrid)[y][x]
				debug := false
				if debug {
					fmt.Printf("Checking cell %v at (%v,%v)\n", cell.getAppearance(), x, y)
				}
				if cell.occupant != nil {
					hum := cell.occupant
					if debug {
						fmt.Println("Acting for", hum)
					}
					if hum.hitpoints <= 0 {
						log.Panic("Zombie!")
					}
					if hum.nextRoundToMove > round {
						// Already moved this humanoid this round
						if debug {
							fmt.Println("Already moved")
						}
						continue
					}
					hum.nextRoundToMove++
					numHumanoids[hum.race]++
					xOpp, yOpp := findNearestOpponent(cellGrid, x, y, debug)
					if debug {
						fmt.Printf("Nearest opponent is at (%v,%v)\n", xOpp, yOpp)
					}
					if xOpp < 0 && yOpp < 0 {
						// No reachable opponent found
						continue
					}
					xCurr, yCurr := x, y
					xStep, yStep := findShortestRoute(
						cellGrid, x, y, xOpp, yOpp, debug)
					if debug {
						fmt.Printf("xstep: %v, ystep: %v\n", xStep, yStep)
					}
					if xStep != xOpp || yStep != yOpp {
						if debug {
							fmt.Printf("Moving to %v at (%v,%v)\n",
								(*cellGrid)[yStep][xStep].getAppearance(), xStep, yStep)
						}
						hum.moveTo((*cellGrid)[yStep][xStep])
						xCurr, yCurr = xStep, yStep
					}
					opponent := findWeakestNeighboringOpponent(
						cellGrid, xCurr, yCurr)
					if opponent != nil {
						hum.attack(opponent)
						if debug {
							fmt.Println("Attacked", hum, opponent)
						}
						if opponent.hitpoints <= 0 {
							if numHumanoids[opponent.race] > 0 {
								numHumanoids[opponent.race]--
							}
							if opponent.race == ELF_SYMBOL {
								if debug {
									fmt.Printf("Elf died at (%v,%v)\n",
										xOpp, yOpp)
								}
								didAnyElvesDie = true
								if abortOnElfDeath {
									if debug {
										fmt.Println("Aborting early due to elf death")
									}
									stop = true
									y = len(*cellGrid)
									break
								}
							}
						}
					}
				}
			}
		}
		// fmt.Println("End of round", round)
		// printCellGrid(cellGrid)
		if !stop {
			// Exit if either race is all dead
			for _, numLeft := range numHumanoids {
				if numLeft <= 0 {
					stop = true
				}
			}
		}
	}

	// Add up hitpoints of remaining humanoids
	totalHitpoints := 0
	for y := 0; y < len(*cellGrid); y++ {
		for x := 0; x < len((*cellGrid)[y]); x++ {
			cell := (*cellGrid)[y][x]
			if cell.occupant != nil {
				// fmt.Println(cell.occupant.hitpoints)
				totalHitpoints += cell.occupant.hitpoints
			}
		}
	}
	outcome := totalHitpoints * (round - 1)
	return outcome, didAnyElvesDie
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	// Create grid of cells
	cellGrid := make([][]*Cell, 0)
	for y := 0; scanner.Scan(); y++ {
		newLine := scanner.Text()
		gridRow := make([]*Cell, len(newLine))
		for x, r := range newLine {
			gridRow[x] = newCell(Symbol(r), ATTACK_POWER)

		}
		cellGrid = append(cellGrid, gridRow)
	}
	// printCellGrid(&cellGrid)

	elfAttackPower := ATTACK_POWER
	maxElfAttackPowerForAnyElvesToDie := ATTACK_POWER
	minElfAttackPowerForNoElvesToDie := 1000
	var outcome int
	var didAnyElvesDie bool
	for elfAttackPower != minElfAttackPowerForNoElvesToDie {
		// fmt.Println("Simulating elf attack power of", elfAttackPower)
		cellGridCopy := copyCellGrid(&cellGrid)
		setElfAttackPower(&cellGridCopy, elfAttackPower)
		outcome, didAnyElvesDie = simulateBattle(
			&cellGridCopy, elfAttackPower != ATTACK_POWER)
		// fmt.Println("All elves survive?", !didAnyElvesDie)
		if elfAttackPower == ATTACK_POWER {
			fmt.Println("PART 1")
			fmt.Printf("Battle outcome for elf attack power of %v: %v\n",
				elfAttackPower, outcome)
		}
		if didAnyElvesDie && elfAttackPower > maxElfAttackPowerForAnyElvesToDie {
			maxElfAttackPowerForAnyElvesToDie = elfAttackPower
		} else if !didAnyElvesDie && elfAttackPower < minElfAttackPowerForNoElvesToDie {
			minElfAttackPowerForNoElvesToDie = elfAttackPower
		}
		elfAttackPower = (maxElfAttackPowerForAnyElvesToDie+minElfAttackPowerForNoElvesToDie)/2 + 1
	}
	fmt.Println("PART 2")
	fmt.Printf("Battle outcome for elf attack power of %v: %v\n",
		elfAttackPower, outcome)
}
