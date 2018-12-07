package main

import (
	"bufio"
	"fmt"
	"log"
	"math"
	"os"
	"regexp"
	"strconv"
)

type Point struct {
	x, y int
}

func distance(p1, p2 Point) int {
	// Manhattan distance
	return int(math.Abs(float64(p1.x-p2.x)) + math.Abs(float64(p1.y-p2.y)))
}

type Coordinate struct {
	center          Point
	area            int
	nextSpiralPoint Point
	spiralRadius    int
}

type GridCell struct {
	nearestCoord *Coordinate
	distance     int
}

func (coord *Coordinate) spiralOutward() Point {
	// Counter-clockwise spiral out of point A
	// v<<<^
	// vv<^^
	// vvA^^
	// v>>^^
	// v>>>^
	previousSpiralPoint := coord.nextSpiralPoint
	var nextSpiralPoint Point

	// Check whether we're at a corner
	if previousSpiralPoint.x == coord.center.x+coord.spiralRadius &&
		previousSpiralPoint.y == coord.center.y+coord.spiralRadius {
		// Top-left corner: increase radius and continue moving in +y direction
		coord.spiralRadius++
		nextSpiralPoint = Point{previousSpiralPoint.x, previousSpiralPoint.y + 1}
	} else if previousSpiralPoint.x == coord.center.x-coord.spiralRadius &&
		previousSpiralPoint.y == coord.center.y+coord.spiralRadius {
		// Top-left corner: start moving in -y direction
		nextSpiralPoint = Point{previousSpiralPoint.x, previousSpiralPoint.y - 1}
	} else if previousSpiralPoint.x == coord.center.x-coord.spiralRadius &&
		previousSpiralPoint.y == coord.center.y-coord.spiralRadius {
		// Bottom-left corner: start moving in +x direction
		nextSpiralPoint = Point{previousSpiralPoint.x + 1, previousSpiralPoint.y}
	} else if previousSpiralPoint.x == coord.center.x+coord.spiralRadius &&
		previousSpiralPoint.y == coord.center.y-coord.spiralRadius {
		// Bottom-right corner: start moving in +y direction
		nextSpiralPoint = Point{previousSpiralPoint.x, previousSpiralPoint.y + 1}

		// If we're not at a corner, figure out what side we're on
	} else if previousSpiralPoint.x == coord.center.x+coord.spiralRadius {
		// Right edge: continue moving in +y direction
		nextSpiralPoint = Point{previousSpiralPoint.x, previousSpiralPoint.y + 1}
	} else if previousSpiralPoint.y == coord.center.y+coord.spiralRadius {
		// Top edge: continue moving in -x direction
		nextSpiralPoint = Point{previousSpiralPoint.x - 1, previousSpiralPoint.y}
	} else if previousSpiralPoint.x == coord.center.x-coord.spiralRadius {
		// Left edge: continue moving in -y direction
		nextSpiralPoint = Point{previousSpiralPoint.x, previousSpiralPoint.y - 1}
	} else if previousSpiralPoint.y == coord.center.y-coord.spiralRadius {
		// Bottom edge: continue moving in +x direction
		nextSpiralPoint = Point{previousSpiralPoint.x + 1, previousSpiralPoint.y}
	} else {
		log.Fatal("Spiral error for Coordinate", coord)
	}

	coord.nextSpiralPoint = nextSpiralPoint
	return previousSpiralPoint
}

type BoundingBox struct {
	min, max Point
}

func (bbox *BoundingBox) update(p Point) {
	if p.x < bbox.min.x {
		bbox.min.x = p.x
	} else if p.x > bbox.max.x {
		bbox.max.x = p.x
	}
	if p.y < bbox.min.y {
		bbox.min.y = p.y
	} else if p.y > bbox.max.y {
		bbox.max.y = p.y
	}
}

func (bbox *BoundingBox) contains(p Point) bool {
	return (p.x >= bbox.min.x && p.y >= bbox.min.y &&
		p.x <= bbox.max.x && p.y <= bbox.max.y)
}

func toInt(s string) int {
	num, err := strconv.Atoi(s)
	if err != nil {
		panic(err)
	}
	return num
}

func parseCoordinate(s string, re *regexp.Regexp) Coordinate {
	matches := re.FindStringSubmatch(s)
	if matches == nil {
		log.Fatal("No matches found for %v", s)
	}
	if len(matches) != 3 {
		log.Fatal("Expected 3 matches but found %v", matches)
	}
	x, y := toInt(matches[1]), toInt(matches[2])
	return Coordinate{Point{x, y}, 0, Point{x, y}, 0}
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

	reader := bufio.NewReader(file)
	scanner := bufio.NewScanner(reader)

	coordinates := make([]*Coordinate, 0)
	bbox := BoundingBox{Point{1000, 1000}, Point{-1000, -1000}}

	coordinateRegex := regexp.MustCompile("([0-9]+), ([0-9]+)")

	for scanner.Scan() {
		newLine := scanner.Text()
		newCoord := parseCoordinate(newLine, coordinateRegex)
		coordinates = append(coordinates, &newCoord)
		bbox.update(newCoord.center)
	}

	if bbox.min.x < 0 || bbox.min.y < 0 {
		log.Fatal("Bounding box extends into negative territory")
	}

	grid := make([][]*GridCell, bbox.max.x+1)
	for x := range grid {
		grid[x] = make([]*GridCell, bbox.max.y+1)
	}

	for _, coord := range coordinates {
		lastUpdatedRadius := 0
		for coord.spiralRadius <= lastUpdatedRadius+1 {
			currPoint := coord.spiralOutward()
			if !bbox.contains(currPoint) {
				continue
			}
			dist := distance(currPoint, coord.center)
			if grid[currPoint.x][currPoint.y] == nil {
				grid[currPoint.x][currPoint.y] = &GridCell{nil, 0}
			}
			cell := grid[currPoint.x][currPoint.y]
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
					log.Fatal("Returned to cell multiple times")
				}
				if cell.nearestCoord != nil {
					cell.nearestCoord.area--
					cell.nearestCoord = nil
				}
			}
		}
	}

	// Loop around edges and discard adjacent coords
	for x := bbox.min.x; x <= bbox.max.x; x++ {
		if grid[x][bbox.min.y].nearestCoord != nil {
			grid[x][bbox.min.y].nearestCoord.area = -1
		}
		if grid[x][bbox.max.y].nearestCoord != nil {
			grid[x][bbox.max.y].nearestCoord.area = -1
		}
	}
	for y := bbox.min.y; y <= bbox.max.y; y++ {
		if grid[bbox.min.x][y].nearestCoord != nil {
			grid[bbox.min.x][y].nearestCoord.area = -1
		}
		if grid[bbox.max.x][y].nearestCoord != nil {
			grid[bbox.max.x][y].nearestCoord.area = -1
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
	for x := bbox.min.x; x <= bbox.max.x; x++ {
		for y := bbox.min.y; y <= bbox.max.y; y++ {
			summedDistance := 0
			for _, coord := range coordinates {
				summedDistance += distance(Point{x, y}, coord.center)
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
