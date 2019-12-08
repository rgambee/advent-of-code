package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
)

type Coordinates struct {
	w, x, y, z int
}

func coordsFromSlice(slice []int) Coordinates {
	return Coordinates{slice[0], slice[1], slice[2], slice[3]}
}

func (c1 *Coordinates) distanceTo(c2 Coordinates) int {
	return (utils.AbsInt(c1.w-c2.w) + utils.AbsInt(c1.x-c2.x) +
		utils.AbsInt(c1.y-c2.y) + utils.AbsInt(c1.z-c2.z))
}

type Constellation []Coordinates

func (con *Constellation) includesCoords(coords Coordinates) bool {
	for _, conCoords := range *con {
		if coords.distanceTo(conCoords) <= 3 {
			return true
		}
	}
	return false
}

func (con *Constellation) addCoordinates(coords Coordinates) {
	*con = append(*con, coords)
}

func (con1 *Constellation) join(con2 *Constellation) {
	*con1 = append(*con1, *con2...)
	*con2 = nil
}

func haveCoordinatesInCommon(con1, con2 *Constellation) bool {
	for _, coords1 := range *con1 {
		for _, coords2 := range *con2 {
			if coords1 == coords2 {
				return true
			}
		}
	}
	return false
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	coordRE := regexp.MustCompile("(-?\\d+),(-?\\d+),(-?\\d+),(-?\\d+)")
	coordinates := make([]Coordinates, 0)
	for scanner.Scan() {
		coordsStr, err := utils.ParseString(scanner.Text(), coordRE, 4)
		if err != nil {
			panic(err)
		}
		coordinates = append(coordinates, coordsFromSlice(
			*utils.StringSliceToIntSlice(&coordsStr)))
	}
	// fmt.Println("Number of coordinates:", len(coordinates))

	constellations := make([]Constellation, 0)
	for _, coords := range coordinates {
		// fmt.Println("Considering coordinates", coords)
		constellationsThatIncludeTheseCoords := make([]int, 0)
		for j, con := range constellations {
			if con.includesCoords(coords) {
				constellationsThatIncludeTheseCoords = append(
					constellationsThatIncludeTheseCoords, j)
			}
		}
		if len(constellationsThatIncludeTheseCoords) == 0 {
			newConstellation := make(Constellation, 1)
			newConstellation[0] = coords
			constellations = append(constellations, newConstellation)
		} else if len(constellationsThatIncludeTheseCoords) == 1 {
			constellations[constellationsThatIncludeTheseCoords[0]].addCoordinates(coords)
		} else {
			// Coordinates belong to multiple constellations,
			// so join them into one constellation
			joinedConstellation := constellations[constellationsThatIncludeTheseCoords[0]]
			for j := len(constellationsThatIncludeTheseCoords) - 1; j > 0; j-- {
				conToJoin := constellationsThatIncludeTheseCoords[j]
				joinedConstellation.join(&constellations[conToJoin])
				// Delete partial constellation
				constellations = append(constellations[:conToJoin], constellations[conToJoin+1:]...)
			}
			joinedConstellation.addCoordinates(coords)
			constellations[constellationsThatIncludeTheseCoords[0]] = joinedConstellation
		}
	}
	constellationSizeSum := 0
	for _, con := range constellations {
		// fmt.Println("Size of constellation:", len(con))
		constellationSizeSum += len(con)
	}
	if constellationSizeSum != len(coordinates) {
		log.Panicf("Combined constellation size (%v) != number of coordinates (%v)",
			constellationSizeSum, len(coordinates))
	}
	fmt.Println("PART 1")
	fmt.Println("Number of constellations is", len(constellations))
}
