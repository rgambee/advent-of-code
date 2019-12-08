package main

import (
	"bufio"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"image"
	"log"
)

type Direction rune

func (dir Direction) String() string {
	return string(dir)
}

const (
	NORTH              Direction = 'N'
	EAST               Direction = 'E'
	SOUTH              Direction = 'S'
	WEST               Direction = 'W'
	DISTANCE_THRESHOLD           = 1000
)

type Room struct {
	doors map[Direction]bool
}

func newRoom() Room {
	return Room{make(map[Direction]bool)}
}

type Coordinates image.Point

func (coords Coordinates) step(dir Direction) Coordinates {
	if dir == NORTH {
		return Coordinates{coords.X, coords.Y + 1}
	} else if dir == EAST {
		return Coordinates{coords.X + 1, coords.Y}
	} else if dir == SOUTH {
		return Coordinates{coords.X, coords.Y - 1}
	} else if dir == WEST {
		return Coordinates{coords.X - 1, coords.Y}
	}
	log.Panic("Unknown direction", string(dir))
	return Coordinates{}
}

func isDirection(r rune) bool {
	dir := Direction(r)
	return dir == NORTH || dir == EAST || dir == SOUTH || dir == WEST
}

func getOppositeDirection(dir Direction) Direction {
	if dir == NORTH {
		return SOUTH
	} else if dir == EAST {
		return WEST
	} else if dir == SOUTH {
		return NORTH
	} else if dir == WEST {
		return EAST
	}
	log.Panicf("Invalid direction %v", dir)
	return 0
}

func addRooms(rooms *map[Coordinates]*Room, startingCoords Coordinates,
	reader *bufio.Reader) (Coordinates, rune) {
	currCoords := startingCoords
	var lastReadRune rune
	var err error
	for {
		lastReadRune, _, err = reader.ReadRune()
		if err != nil {
			panic(err)
		}
		if isDirection(lastReadRune) {
			currRoom := (*rooms)[currCoords]
			dir := Direction(lastReadRune)
			nextCoords := currCoords.step(dir)
			nextRoom, present := (*rooms)[nextCoords]
			if !present {
				newRoom := newRoom()
				nextRoom = &newRoom
				(*rooms)[nextCoords] = nextRoom
			}
			currRoom.doors[dir] = true
			nextRoom.doors[getOppositeDirection(dir)] = true
			currCoords = nextCoords
		} else if lastReadRune == '(' {
			// Start new branch(es)
			for {
				_, lastReadRune := addRooms(
					rooms, currCoords, reader)
				if lastReadRune == ')' {
					// Done adding branches
					break
				} else if lastReadRune != '|' {
					log.Panic("Unexpected input symbol", string(lastReadRune))
				}
			}
		} else if lastReadRune == '^' {
			// Skip beginning character
			continue
		} else if lastReadRune == '|' || lastReadRune == ')' || lastReadRune == '$' {
			// End of branch
			break
		} else {
			log.Panic("Unrecognized input symbol ", string(lastReadRune))
		}
	}
	return currCoords, lastReadRune
}

func findDistances(rooms *map[Coordinates]*Room,
	startingCoords Coordinates, distanceThreshold int) (int, int) {
	distances := make(map[Coordinates]int)
	furthestCoords := make([]Coordinates, 1)
	furthestCoords[0] = startingCoords
	numRoomsBeyondThreshold := 0
	dist := 0
	for ; len(furthestCoords) > 0; dist++ {
		coordsToCheckNext := make([]Coordinates, 0)
		for _, coords := range furthestCoords {
			distances[coords] = dist
			if dist >= distanceThreshold {
				numRoomsBeyondThreshold++
			}
			room := (*rooms)[coords]
			for dir, _ := range room.doors {
				nextCoords := coords.step(dir)
				if _, present := distances[nextCoords]; !present {
					coordsToCheckNext = append(
						coordsToCheckNext, nextCoords)
				}
			}
		}
		furthestCoords = coordsToCheckNext
	}
	return dist - 1, numRoomsBeyondThreshold
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	reader := bufio.NewReader(file)

	// Construct map
	rooms := make(map[Coordinates]*Room)
	startingCoords := Coordinates{0, 0}
	startingRoom := newRoom()
	rooms[startingCoords] = &startingRoom
	_, lastReadRune := addRooms(&rooms, startingCoords, reader)
	if lastReadRune != '$' {
		log.Panic("Unexpected final rune ", string(lastReadRune))
	}

	// Find furthest room and number of rooms beyond distance threshold
	furthestDistance, numRoomsBeyondThreshold := findDistances(
		&rooms, startingCoords, DISTANCE_THRESHOLD)

	fmt.Println("PART 1")
	fmt.Println("Distance to furthest room is", furthestDistance)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Printf("Number of rooms further than %v: %v\n", DISTANCE_THRESHOLD,
		numRoomsBeyondThreshold)
}
