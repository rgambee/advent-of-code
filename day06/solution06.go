package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
)

type Coordinate struct {
	center          utils.Point2D
	area            int
	nextSpiralPoint utils.Point2D
	spiralRadius    int
}

type GridCell struct {
	nearestCoord *Coordinate
	distance     int
}

func (coord *Coordinate) spiralOutward() utils.Point2D {
	// Counter-clockwise spiral out of point A
	// v<<<^
	// vv<^^
	// vvA^^
	// v>>^^
	// v>>>^
	previousSpiralPoint := coord.nextSpiralPoint
	var nextSpiralPoint utils.Point2D

	// Check whether we're at a corner
	if previousSpiralPoint.X == coord.center.X+coord.spiralRadius &&
		previousSpiralPoint.Y == coord.center.Y+coord.spiralRadius {
		// Top-left corner: increase radius and continue moving in +y direction
		coord.spiralRadius++
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X, previousSpiralPoint.Y+1)
	} else if previousSpiralPoint.X == coord.center.X-coord.spiralRadius &&
		previousSpiralPoint.Y == coord.center.Y+coord.spiralRadius {
		// Top-left corner: start moving in -y direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X, previousSpiralPoint.Y-1)
	} else if previousSpiralPoint.X == coord.center.X-coord.spiralRadius &&
		previousSpiralPoint.Y == coord.center.Y-coord.spiralRadius {
		// Bottom-left corner: start moving in +x direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X+1, previousSpiralPoint.Y)
	} else if previousSpiralPoint.X == coord.center.X+coord.spiralRadius &&
		previousSpiralPoint.Y == coord.center.Y-coord.spiralRadius {
		// Bottom-right corner: start moving in +y direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X, previousSpiralPoint.Y+1)

		// If we're not at a corner, figure out what side we're on
	} else if previousSpiralPoint.X == coord.center.X+coord.spiralRadius {
		// Right edge: continue moving in +y direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X, previousSpiralPoint.Y+1)
	} else if previousSpiralPoint.Y == coord.center.Y+coord.spiralRadius {
		// Top edge: continue moving in -x direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X-1, previousSpiralPoint.Y)
	} else if previousSpiralPoint.X == coord.center.X-coord.spiralRadius {
		// Left edge: continue moving in -y direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X, previousSpiralPoint.Y-1)
	} else if previousSpiralPoint.Y == coord.center.Y-coord.spiralRadius {
		// Bottom edge: continue moving in +x direction
		nextSpiralPoint = utils.NewPoint2D(
			previousSpiralPoint.X+1, previousSpiralPoint.Y)
	} else {
		log.Panic("Spiral error for Coordinate", coord)
	}

	coord.nextSpiralPoint = nextSpiralPoint
	return previousSpiralPoint
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	coordinates := make([]*Coordinate, 0)
	bbox := utils.NewBoundingBox2D(
		utils.NewPoint2D(1000, 1000), utils.NewPoint2D(-1000, -1000))

	coordinateRegex := regexp.MustCompile("([0-9]+), ([0-9]+)")

	for scanner.Scan() {
		newLine := scanner.Text()
		parsedLine, err := utils.ParseString(newLine, coordinateRegex, 2)
		if err != nil {
			panic(err)
		}
		x, y := utils.StringToInt(parsedLine[0]), utils.StringToInt(parsedLine[1])
		newCoord := Coordinate{utils.NewPoint2D(x, y), 0, utils.NewPoint2D(x, y), 0}
		coordinates = append(coordinates, &newCoord)
		bbox.Update(newCoord.center)
	}

	if bbox.Min.X < 0 || bbox.Min.Y < 0 {
		log.Panic("Bounding box extends into negative territory")
	}

	grid := make([][]*GridCell, bbox.Max.X+1)
	for x := range grid {
		grid[x] = make([]*GridCell, bbox.Max.Y+1)
	}

	for _, coord := range coordinates {
		lastUpdatedRadius := 0
		for coord.spiralRadius <= lastUpdatedRadius+1 {
			currPoint := coord.spiralOutward()
			if !bbox.Contains(currPoint) {
				continue
			}
			dist := currPoint.DistanceTo(coord.center)
			if grid[currPoint.X][currPoint.Y] == nil {
				grid[currPoint.X][currPoint.Y] = &GridCell{nil, 0}
			}
			cell := grid[currPoint.X][currPoint.Y]
			if (cell.nearestCoord == nil && cell.distance == 0) ||
				dist < cell.distance {
				if cell.nearestCoord != nil {
					cell.nearestCoord.area--
				}
				cell.nearestCoord = coord
				cell.distance = dist
				coord.area++
				lastUpdatedRadius = coord.spiralRadius
			} else if dist == cell.distance {
				if coord == cell.nearestCoord {
					log.Panic("Returned to cell multiple times")
				}
				if cell.nearestCoord != nil {
					cell.nearestCoord.area--
					cell.nearestCoord = nil
				}
			}
		}
	}

	// Loop around edges and discard adjacent coords
	for x := bbox.Min.X; x <= bbox.Max.X; x++ {
		if grid[x][bbox.Min.Y].nearestCoord != nil {
			grid[x][bbox.Min.Y].nearestCoord.area = -1
		}
		if grid[x][bbox.Max.Y].nearestCoord != nil {
			grid[x][bbox.Max.Y].nearestCoord.area = -1
		}
	}
	for y := bbox.Min.Y; y <= bbox.Max.Y; y++ {
		if grid[bbox.Min.X][y].nearestCoord != nil {
			grid[bbox.Min.X][y].nearestCoord.area = -1
		}
		if grid[bbox.Max.X][y].nearestCoord != nil {
			grid[bbox.Max.X][y].nearestCoord.area = -1
		}
	}

	largestArea := -1
	for _, c := range coordinates {
		if c.area > largestArea {
			largestArea = c.area
		}
	}

	summedDistanceLimit := 10000
	areaUnderLimit := 0
	for x := bbox.Min.X; x <= bbox.Max.X; x++ {
		for y := bbox.Min.Y; y <= bbox.Max.Y; y++ {
			summedDistance := 0
			for _, coord := range coordinates {
				summedDistance += coord.center.DistanceTo(utils.NewPoint2D(x, y))
			}
			if summedDistance <= summedDistanceLimit {
				areaUnderLimit++
			}
		}
	}

	fmt.Println("PART 1")
	fmt.Println("Largest area", largestArea)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Area of region under summed distance limit:", areaUnderLimit)
}
