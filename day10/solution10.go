package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"image"
	"image/color"
	"image/png"
	"os"
	"regexp"
)

type LightSpot struct {
	position, velocity utils.Point2D
}

func integrate(spots *[]LightSpot, seconds int) *utils.BoundingBox2D {
	var bbox utils.BoundingBox2D
	for i, s := range *spots {
		newPos := utils.NewPoint2D(
			s.position.X+seconds*s.velocity.X,
			s.position.Y+seconds*s.velocity.Y)
		(*spots)[i].position = newPos
		if i == 0 {
			bbox = utils.NewBoundingBox2D(newPos, newPos)
		} else {
			bbox.Update(newPos)
		}
	}
	return &bbox
}

func getAreasAfterIterations(spots *[]LightSpot, secondsToEval []int) []int {
	// Evaluates the bounding box area for each element in secondsToEval
	spotsCopy := make([]LightSpot, len(*spots))
	copy(spotsCopy, *spots)
	areas := make([]int, len(secondsToEval))
	lastSec := 0
	for i, sec := range secondsToEval {
		areas[i] = integrate(&spotsCopy, sec-lastSec).GetArea()
		lastSec = sec
	}
	return areas
}

func minimizeArea(spots *[]LightSpot, gamma float64) int {
	// Uses the gradient descent method
	// https://en.wikipedia.org/wiki/Gradient_descent
	currSec := 0
	for i := 0; true; i++ {
		secsToEval := [3]int{currSec - 1, currSec, currSec + 1}
		areas := getAreasAfterIterations(spots, secsToEval[:])
		areaNm1, areaN, areaNp1 := areas[0], areas[1], areas[2]
		fmt.Printf("Iter %v, currSec %v, area %v\n", i, currSec, areaN)
		if areaN < areaNm1 && areaN < areaNp1 {
			fmt.Printf("Minimum found after %v iterations\n", i)
			break
		}
		currSlope := areaNp1 - areaN
		currSec = int(float64(currSec) - float64(currSlope)*gamma)
	}
	return currSec
}

func saveImage(spots *[]LightSpot, bbox *utils.BoundingBox2D, filename string) {
	// Increase size of rectangle by 1 in each direction since
	// utils.BoundingBox2D includes its Max but image.Rectangle doesn't
	img := image.NewGray(image.Rect(
		bbox.Min.X, bbox.Min.Y, bbox.Max.X+1, bbox.Max.Y+1))
	for _, s := range *spots {
		img.Set(s.position.X, s.position.Y, color.Gray{255})
	}

	file, err := os.Create(filename)
	if err != nil {
		panic(err)
	}
	defer utils.CloseFile(file)
	if err := png.Encode(file, img); err != nil {
		panic(err)
	}
	fmt.Println("Image saved to", filename)
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	// Example line:
	//position=< 53050, -42120> velocity=<-5,  4>
	re := regexp.MustCompile("position=<[ ]?([-]?[0-9]+), [ ]?([-]?[0-9]+)> velocity=<[ ]?([-]?[0-9]+), [ ]?([-]?[0-9]+)>")

	spots := make([]LightSpot, 0)
	for scanner.Scan() {
		newLine := scanner.Text()
		parsedLine, err := utils.ParseString(newLine, re, 4)
		if err != nil {
			panic(err)
		}
		spotData := *utils.StringSliceToIntSlice(&parsedLine)
		posX, posY, velX, velY := spotData[0], spotData[1], spotData[2], spotData[3]
		newSpot := LightSpot{utils.NewPoint2D(posX, posY),
			utils.NewPoint2D(velX, velY)}
		spots = append(spots, newSpot)
	}

	secondsToMinArea := minimizeArea(&spots, 0.002)
	bbox := integrate(&spots, secondsToMinArea)

	fmt.Println("PART 1")
	saveImage(&spots, bbox, "message.png")
	fmt.Println("PART 2")
	fmt.Printf("Minimum bounding box area after %v seconds\n", secondsToMinArea)
}
