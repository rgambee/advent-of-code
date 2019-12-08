package main

import (
	"container/list"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
)

const (
	ROCKY                  CellType  = 0
	WET                    CellType  = 1
	NARROW                 CellType  = 2
	NEITHER                Equipment = 'N'
	TORCH                  Equipment = 'T'
	CLIMING_GEAR           Equipment = 'C'
	EQUIPMENT_SWITCH_TIME            = 7
	DEPTH                            = 3558
	GEO_INDEX_X_MULTIPLIER           = 16807
	GEO_INDEX_Y_MULTIPLIER           = 48271
	EROSION_DIVSOR                   = 20183
	TARGET_X                         = 15
	TARGET_Y                         = 740
)

var SOURCE utils.Point2D = utils.NewPoint2D(0, 0)

type CellType int

func (ct CellType) isPassableWithEquipment(equip Equipment) bool {
	if ct == ROCKY {
		return equip != NEITHER
	} else if ct == WET {
		return equip != TORCH
	} else if ct == NARROW {
		return equip != CLIMING_GEAR
	}
	log.Panicf("Unknown cell type %v", string(ct))
	return false
}

func getCommonEquipment(ct1, ct2 CellType) Equipment {
	// Assumes ct1 != ct2
	if ct1 == ROCKY || ct2 == ROCKY {
		if ct1 == WET || ct2 == WET {
			return CLIMING_GEAR
		} else if ct1 == NARROW || ct2 == NARROW {
			return TORCH
		}
	} else if ct1 == WET || ct2 == WET {
		if ct1 == NARROW || ct2 == NARROW {
			return NEITHER
		}
	}
	log.Panicf("Can't find common equipment for cell types %v and %v", ct1, ct2)
	return Equipment(-1)
}

func getNeighbors(coords *utils.Point2D) [4]utils.Point2D {
	return [4]utils.Point2D{
		utils.NewPoint2D(coords.X-1, coords.Y),
		utils.NewPoint2D(coords.X, coords.Y-1),
		utils.NewPoint2D(coords.X+1, coords.Y),
		utils.NewPoint2D(coords.X, coords.Y+1),
	}
}

type Cell struct {
	geoIndex     int
	erosionLevel int
	cellType     CellType
}

func (cell *Cell) String() string {
	if cell.cellType == ROCKY {
		return "."
	} else if cell.cellType == WET {
		return "="
	} else if cell.cellType == NARROW {
		return "|"
	}
	return "?"
}

func (cell *Cell) determineErosionLevel(depth int) {
	cell.erosionLevel = (cell.geoIndex + depth) % EROSION_DIVSOR
}

func (cell *Cell) determineCellType() {
	cell.cellType = CellType(cell.erosionLevel % 3)
}

type CellGrid map[utils.Point2D]*Cell

func determineGeoIndex(cellGrid *CellGrid, coords, target utils.Point2D) {
	geoIndex := -1
	if coords == SOURCE || coords == target {
		geoIndex = 0
	} else if coords.Y == 0 {
		geoIndex = coords.X * GEO_INDEX_X_MULTIPLIER
	} else if coords.X == 0 {
		geoIndex = coords.Y * GEO_INDEX_Y_MULTIPLIER
	} else {
		leftCoords := utils.NewPoint2D(coords.X-1, coords.Y)
		left, present := (*cellGrid)[leftCoords]
		if !present {
			left = cellGrid.fillInGrid(leftCoords, target)
		}
		upCoords := utils.NewPoint2D(coords.X, coords.Y-1)
		up, present := (*cellGrid)[upCoords]
		if !present {
			up = cellGrid.fillInGrid(upCoords, target)
		}
		geoIndex = left.erosionLevel * up.erosionLevel
	}
	(*cellGrid)[coords].geoIndex = geoIndex
}

func (cellGrid *CellGrid) fillInGrid(coords, target utils.Point2D) *Cell {
	newCell := Cell{}
	(*cellGrid)[coords] = &newCell
	determineGeoIndex(cellGrid, coords, target)
	newCell.determineErosionLevel(DEPTH)
	newCell.determineCellType()
	return &newCell
}

func (cellGrid *CellGrid) getCumulativeRiskLevel(target utils.Point2D) int {
	riskLevel := 0
	for x := 0; x <= target.X; x++ {
		for y := 0; y <= target.Y; y++ {
			riskLevel += int((*cellGrid)[utils.NewPoint2D(x, y)].cellType)
		}
	}
	return riskLevel
}

type Equipment rune

func (equip Equipment) String() string {
	if equip == NEITHER || equip == TORCH || equip == CLIMING_GEAR {
		return string(equip)
	}
	return "?"
}

type RouteNode struct {
	travelTime  int
	equipment   Equipment
	coordinates utils.Point2D
	parent      *RouteNode
}

func (rn *RouteNode) estimateTimeToTarget(target *utils.Point2D) int {
	// Heuristic for A* search algorithm
	return rn.travelTime + rn.coordinates.DistanceTo(*target)
}

type RouteNodeList struct {
	*list.List
}

func NewRouteNodeList() RouteNodeList {
	newList := list.New()
	return RouteNodeList{newList}
}

func (rnl *RouteNodeList) insert(rn *RouteNode, target *utils.Point2D) {
	elemToInsertBefore := rnl.Front()
	timeToTarget := rn.estimateTimeToTarget(target)
	for elemToInsertBefore != nil &&
		elemToInsertBefore.Value.(*RouteNode).estimateTimeToTarget(target) <= timeToTarget {
		elemToInsertBefore = elemToInsertBefore.Next()
	}
	if elemToInsertBefore != nil {
		rnl.InsertBefore(rn, elemToInsertBefore)
	} else {
		rnl.PushBack(rn)
	}
}

func findFastestTime(cellGrid *CellGrid, target utils.Point2D) int {
	// This is a slightly messy implementation of the A* search algorithm
	// https://en.wikipedia.org/wiki/A*_search_algorithm
	routeTree := RouteNode{
		travelTime: 0, equipment: TORCH, coordinates: SOURCE}
	fastestRoutes := make(map[utils.Point2D]map[Equipment]*RouteNode)
	fastestRoutes[SOURCE] = make(map[Equipment]*RouteNode)
	fastestRoutes[SOURCE][TORCH] = &routeTree
	leaves := NewRouteNodeList()
	leaves.PushFront(&routeTree)
	visitedCoords := make(map[utils.Point2D]bool)
	fastestTimeToTarget := -1
	for i := 0; true; i++ {
		currLeaf := leaves.Front()
		leaves.Remove(currLeaf)
		currNode := currLeaf.Value.(*RouteNode)
		for _, neigh := range getNeighbors(&currNode.coordinates) {
			if neigh.X < 0 || neigh.Y < 0 {
				continue
			}
			if _, present := visitedCoords[neigh]; present {
				continue
			}
			time := currNode.travelTime + 1
			equip := currNode.equipment
			// If the grid doesn't include these coordinates (because
			// we're looking beyond the target), extend the grid.
			if _, present := (*cellGrid)[neigh]; !present {
				cellGrid.fillInGrid(neigh, target)
			}
			if !(*cellGrid)[neigh].cellType.isPassableWithEquipment(equip) {
				equip = getCommonEquipment(
					(*cellGrid)[currNode.coordinates].cellType,
					(*cellGrid)[neigh].cellType)
				time += EQUIPMENT_SWITCH_TIME
			}
			newNode := RouteNode{travelTime: time, equipment: equip,
				coordinates: neigh, parent: currNode}
			routeStep, present := fastestRoutes[neigh]
			if !present {
				fastestRoutes[neigh] = make(map[Equipment]*RouteNode)
				routeStep = fastestRoutes[neigh]
			}
			if routeStep[equip] == nil || newNode.travelTime < routeStep[equip].travelTime {
				fastestRoutes[neigh][equip] = &newNode
				for _, e := range [3]Equipment{NEITHER, TORCH, CLIMING_GEAR} {
					if routeStep[e] == nil || routeStep[e].travelTime > newNode.travelTime+7 {
						newChildNode := RouteNode{
							travelTime: newNode.travelTime + 7,
							equipment:  e, coordinates: neigh, parent: &newNode}
						fastestRoutes[neigh][e] = &newChildNode
					}
				}
				if neigh != target {
					leaves.insert(&newNode, &target)
				} else {
					fastestTimeToTarget = fastestRoutes[target][TORCH].travelTime
					fmt.Println("Fastest time to target so far is",
						fastestTimeToTarget)
					fmt.Println("Starting iteration", i)
					fmt.Println("Number of leaves is", leaves.Len())
					// Found a new fastest route to the target,
					// so don't exit yet
					// done = false
				}
			}
		}
	}
	if fastestTimeToTarget < 0 {
		log.Panic("Couldn't find fastest route between source and target")
	}
	return fastestTimeToTarget
}

func printCellGrid(cellGrid *CellGrid, target utils.Point2D) {
	for y := 0; y <= target.Y+5; y++ {
		for x := 0; x <= target.X+5; x++ {
			cell, present := (*cellGrid)[utils.NewPoint2D(x, y)]
			if !present {
				cell = &Cell{0, 0, -1}
			}
			fmt.Printf("%v", cell)
		}
		fmt.Println()
	}
}

func main() {
	target := utils.NewPoint2D(TARGET_X, TARGET_Y)
	cellGrid := make(CellGrid)
	cellGrid.fillInGrid(utils.NewPoint2D(TARGET_X+1, TARGET_Y+1), target)
	cellGrid.fillInGrid(SOURCE, target)
	// printCellGrid(&cellGrid, target)
	riskLevel := cellGrid.getCumulativeRiskLevel(target)
	travelTime := findFastestTime(&cellGrid, target)

	fmt.Println("PART 1")
	fmt.Println("Risk level is", riskLevel)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Fastest time to target is", travelTime)
}
