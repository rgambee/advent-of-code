package utils

import (
	"bufio"
	"image"
	"log"
	"math"
	"os"
	"regexp"
	"strconv"
)

func GetFilename() *string {
	if len(os.Args) < 2 {
		log.Panic("Must provide path to input file")
	}
	filename := os.Args[1]
	return &filename
}

func OpenFile(filename *string) *os.File {
	file, err := os.Open(*filename)
	if err != nil {
		panic(err)
	}
	return file
}

func CloseFile(file *os.File) {
	if err := file.Close(); err != nil {
		panic(err)
	}
}

func GetLineScanner(file *os.File) *bufio.Scanner {
	return GetScanner(file, bufio.ScanLines)
}

func GetWordScanner(file *os.File) *bufio.Scanner {
	return GetScanner(file, bufio.ScanWords)
}

func GetScanner(file *os.File, split bufio.SplitFunc) *bufio.Scanner {
	reader := bufio.NewReader(file)
	scanner := bufio.NewScanner(reader)
	scanner.Split(split)
	return scanner
}

func ParseString(s string, re *regexp.Regexp, expectedMatches int) []string {
	matches := re.FindStringSubmatch(s)
	if matches == nil {
		log.Fatal("No matches found for ", s)
	}
	// First element of matches is the match for the entire regexp,
	// which we don't care about
	if len(matches) != expectedMatches+1 {
		log.Fatal("Expected %v matches but found %v", expectedMatches, len(matches)-1)
	}
	return matches[1:]
}

func StringToInt(s string) int {
	num, err := strconv.Atoi(s)
	if err != nil {
		panic(err)
	}
	return num
}

func StringSliceToIntSlice(stringSlice *[]string) *[]int {
	intSlice := make([]int, len(*stringSlice))
	for i, s := range *stringSlice {
		intSlice[i] = StringToInt(s)
	}
	return &intSlice
}

func SumSlice(slice *[]int) int {
	total := 0
	for _, n := range *slice {
		total += n
	}
	return total
}

func FindSliceMax(slice *[]int) (index, max int) {
	index = -1
	max = -(1 << 31)
	for i, elem := range *slice {
		if elem > max {
			max = elem
			index = i
		}
	}
	return
}

func FindMax(nums ...int) int {
	_, max := FindSliceMax(&nums)
	return max
}

func FindSliceMin(slice *[]int) (index, min int) {
	index = -1
	min = 1<<31 - 1
	for i, elem := range *slice {
		if elem < min {
			min = elem
			index = i
		}
	}
	return
}

func FindMin(nums ...int) int {
	_, min := FindSliceMin(&nums)
	return min
}

func MakeSquareIntSlice(size int) *[][]int {
	return MakeRectIntSlice(size, size)
}

func MakeRectIntSlice(size0, size1 int) *[][]int {
	slice := make([][]int, size0)
	for i := range slice {
		slice[i] = make([]int, size1)
	}
	return &slice
}

func AbsInt(n int) int {
	return int(math.Abs(float64(n)))
}

type Point2D struct {
	image.Point
}

func NewPoint2D(x, y int) Point2D {
	return Point2D{image.Point{x, y}}
}

func (p1 *Point2D) DistanceTo(p2 Point2D) int {
	// Manhattan distance
	return AbsInt(p1.X-p2.X) + AbsInt(p1.Y-p2.Y)
}

type BoundingBox2D struct {
	*image.Rectangle
}

func NewBoundingBox2D(min, max Point2D) BoundingBox2D {
	return BoundingBox2D{&image.Rectangle{
		image.Point{min.X, min.Y},
		image.Point{max.X, max.Y}}}
}

func (bbox *BoundingBox2D) Update(p Point2D) {
	// Inclusive at both min and max (unlike image.Rectangle)
	if p.X < bbox.Min.X {
		bbox.Min.X = p.X
	} else if p.X > bbox.Max.X {
		bbox.Max.X = p.X
	}
	if p.Y < bbox.Min.Y {
		bbox.Min.Y = p.Y
	} else if p.Y > bbox.Max.Y {
		bbox.Max.Y = p.Y
	}
}

func (bbox *BoundingBox2D) GetArea() int {
	return bbox.Dx() * bbox.Dy()
}

func (bbox *BoundingBox2D) Contains(p Point2D) bool {
	// Inclusive at both min and max (unlike image.Rectangle)
	return (p.X >= bbox.Min.X && p.Y >= bbox.Min.Y &&
		p.X <= bbox.Max.X && p.Y <= bbox.Max.Y)
}
