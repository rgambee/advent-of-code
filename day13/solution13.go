package main

import (
	"errors"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"image"
	"log"
)

// TODO: enum (also for track type)
var INTERSECTION_DIRECTIONS [3]int = [3]int{0, 1, 2}

type Track struct {
	ascii       rune
	neighbors   map[image.Point]*Track
	minecart    *Minecart
	coordinates image.Point
}

type Minecart struct {
	velocity                  image.Point
	nextIntersectionDirection int
	moveOnTick                int
}

func rotateCW(p image.Point) image.Point {
	return image.Point{-p.Y, p.X}
}

func rotateACW(p image.Point) image.Point {
	return image.Point{p.Y, -p.X}
}

func (m *Minecart) turnCW() {
	m.velocity = rotateCW(m.velocity)
}

func (m *Minecart) turnACW() {
	m.velocity = rotateACW(m.velocity)
}

func findTrackAtCoordinates(coords image.Point, tracks *[]*Track) (*Track, error) {
	// TODO: take advantage of fact that tracks is sorted
	for _, track := range *tracks {
		if track.coordinates == coords {
			return track, nil
		}
	}
	return nil, errors.New(fmt.Sprintf("Track not found at coordinates %v", coords))
}

func addNewTrack(r rune, coords image.Point, tracks *[]*Track) {
	newTrack := Track{coordinates: coords, neighbors: make(map[image.Point]*Track)}
	// TODO: clean this up and reduce reused code
	// TODO: check number of neighbors ('+' == 4, else 2)
	// TODO: check track boundaries
	if !(r == '-' || r == '>' || r == '<') {
		// Need an up neighbor
		direction := image.Point{0, -1}
		upNeighbor, err := findTrackAtCoordinates(coords.Add(direction), tracks)
		if r == '/' || r == '\\' {
			if upNeighbor != nil && !(upNeighbor.ascii == '|' || upNeighbor.ascii == '+') {
				upNeighbor = nil
			}
		} else {
			if err != nil {
				panic(err)
			}
		}
		if upNeighbor != nil {
			newTrack.neighbors[direction] = upNeighbor
			// Add new track as down neighbor to existing track
			upNeighbor.neighbors[image.Point{-direction.X, -direction.Y}] = &newTrack
		}

		if r == '^' {
			newTrack.minecart = &Minecart{image.Point{0, -1}, 0, 0}
			newTrack.ascii = '|'
		} else if r == 'v' {
			newTrack.minecart = &Minecart{image.Point{0, 1}, 0, 0}
			newTrack.ascii = '|'
		} else {
			newTrack.ascii = r
		}
	}

	if !(r == '|' || r == '^' || r == 'v') {
		// Need a left neighbor
		direction := image.Point{-1, 0}
		leftNeighbor, err := findTrackAtCoordinates(coords.Add(direction), tracks)
		if r == '/' || r == '\\' {
			if leftNeighbor != nil && !(leftNeighbor.ascii == '-' || leftNeighbor.ascii == '+') {
				leftNeighbor = nil
			}
		} else {
			if err != nil {
				panic(err)
			}
		}
		if leftNeighbor != nil {
			newTrack.neighbors[direction] = leftNeighbor
			// Add new track as right neighbor to existing track
			leftNeighbor.neighbors[image.Point{-direction.X, -direction.Y}] = &newTrack
		}

		if r == '>' {
			newTrack.minecart = &Minecart{image.Point{1, 0}, 0, 0}
			newTrack.ascii = '-'
		} else if r == '<' {
			newTrack.minecart = &Minecart{image.Point{-1, 0}, 0, 0}
			newTrack.ascii = '-'
		} else {
			newTrack.ascii = r
		}
	}
	*tracks = append(*tracks, &newTrack)
}

func countMinecarts(tracks *[]*Track) (int, image.Point) {
	numMinecarts := 0
	location := image.Point{}
	for _, track := range *tracks {
		if track.minecart != nil {
			numMinecarts++
			location = track.coordinates
			// fmt.Println(track.coordinates, string(track.ascii))
		}
	}
	return numMinecarts, location
}

func moveMinecart(startingTrack *Track, tracks *[]*Track, reportCollision bool) bool {
	// TODO: reduce reused code
	// TODO: check track type and minecart behavior (-| == straight, /\ == turn)
	minecart := startingTrack.minecart
	if minecart == nil {
		log.Panic("Can't move nonexistant minecart")
	}
	// fmt.Println("Moving minecart at", startingTrack)
	collision := false
	destinationTrack := &Track{}
	if startingTrack.ascii == '+' {
		// fmt.Println("Entering intersection at", startingTrack.coordinates)
		if minecart.nextIntersectionDirection == 0 {
			// Turn left (counter-/anti-clockwise)
			// fmt.Println("Turning left at intersection", startingTrack.coordinates)
			destinationTrack = startingTrack.neighbors[rotateACW(minecart.velocity)]
			minecart.turnACW()
		} else if minecart.nextIntersectionDirection == 1 {
			// Head straight
			destinationTrack = startingTrack.neighbors[minecart.velocity]
		} else if minecart.nextIntersectionDirection == 2 {
			// Turn right (clockwise)
			destinationTrack = startingTrack.neighbors[rotateCW(minecart.velocity)]
			minecart.turnCW()
		} else {
			log.Panic("Invalid minecart intersection behavior", startingTrack, minecart)
		}
		minecart.nextIntersectionDirection = (minecart.nextIntersectionDirection + 1) % len(INTERSECTION_DIRECTIONS)
	} else {
		if startingTrack.neighbors[minecart.velocity] != nil {
			// Move minecart straight ahead
			if startingTrack.ascii != '|' && startingTrack.ascii != '-' {
				log.Panic("Going straight on curved track", startingTrack)
			}
			destinationTrack = startingTrack.neighbors[minecart.velocity]
		} else {
			// Turn minecart
			if startingTrack.ascii != '/' && startingTrack.ascii != '\\' {
				log.Panic("Turning on straight track", startingTrack)
			}
			if startingTrack.neighbors[rotateCW(minecart.velocity)] != nil {
				destinationTrack = startingTrack.neighbors[rotateCW(minecart.velocity)]
				minecart.turnCW()
			} else if startingTrack.neighbors[rotateACW(minecart.velocity)] != nil {
				destinationTrack = startingTrack.neighbors[rotateACW(minecart.velocity)]
				minecart.turnACW()
			} else {
				log.Panic("Minecart has nowhere to go")
			}
		}
	}
	if destinationTrack.ascii == 0 {
		log.Panic("Couldn't figure out where minecart should go")
	} else {
		if destinationTrack.minecart == nil {
			minecart.moveOnTick++
			destinationTrack.minecart = minecart
			startingTrack.minecart = nil
		} else {
			if reportCollision {
				fmt.Println("PART 1")
				fmt.Println("Minecart collision at coordinates",
					destinationTrack.coordinates)
			}
			startingTrack.minecart = nil
			destinationTrack.minecart = nil
			collision = true
		}
	}
	// fmt.Printf("Minecart moved from %v to %v\n", startingTrack.coordinates,
	// 	destinationTrack.coordinates)
	return collision
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	tracks := make([]*Track, 0)
	for y := 0; scanner.Scan(); y++ {
		for x, r := range scanner.Text() {
			if r != ' ' {
				addNewTrack(r, image.Point{x, y}, &tracks)
			}
		}
	}

	// Check track network validity
	for _, track := range tracks {
		if track.ascii == '+' && len(track.neighbors) != 4 {
			log.Panic("Invalid intersection", track)
		} else if track.ascii != '+' && len(track.neighbors) != 2 {
			log.Panic("Invalid track", track)
		}
	}

	firstCollisionHasOccurred := false
	for tick := 0; true; tick++ {
		// fmt.Println("Tick", tick)
		for _, track := range tracks {
			if track.minecart != nil && track.minecart.moveOnTick <= tick {
				firstCollisionHasOccurred = moveMinecart(track, &tracks,
					!firstCollisionHasOccurred) || firstCollisionHasOccurred
			}
		}
		// fmt.Println(findTrackAtCoordinates(image.Point{115, 6}, &tracks))
		minecartsLeft, loc := countMinecarts(&tracks)
		// fmt.Println(minecartsLeft)
		if minecartsLeft < 2 {
			fmt.Println("PART 2")
			fmt.Println("Last minecart at", loc)
			break
		}
	}
}
