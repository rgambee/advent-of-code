package utils

import (
	"bufio"
	"errors"
	"fmt"
	"image"
	"log"
	"math"
	"math/big"
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

func ParseString(s string, re *regexp.Regexp, expectedMatches int) ([]string, error) {
	matches := re.FindStringSubmatch(s)
	if matches == nil {
		return nil, errors.New(fmt.Sprintf("No matches found for '%v'", s))
	}
	// First element of matches is the match for the entire regexp,
	// which we don't care about
	if len(matches) != expectedMatches+1 {
		return nil, errors.New(fmt.Sprintf(
			"Expected %v matches but found %v", expectedMatches, len(matches)-1))
	}
	return matches[1:], nil
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

type Point3D struct {
	X, Y, Z int
}

func (p1 *Point3D) DistanceTo(p2 Point3D) int {
	return AbsInt(p1.X-p2.X) + AbsInt(p1.Y-p2.Y) + AbsInt(p1.Z-p2.Z)
}

func (p1 *Point3D) Translate(p2 Point3D) {
	p1.X += p2.X
	p1.Y += p2.Y
	p1.Z += p2.Z
}

type BoundingBox3D struct {
	Min, Max Point3D
}

func (bbox *BoundingBox3D) Update(p Point3D) {
	if p.X < bbox.Min.X {
		bbox.Min.X = p.X
	}
	if p.Y < bbox.Min.Y {
		bbox.Min.Y = p.Y
	}
	if p.Z < bbox.Min.Z {
		bbox.Min.Z = p.Z
	}
	if p.X > bbox.Max.X {
		bbox.Max.X = p.X
	}
	if p.Y > bbox.Max.Y {
		bbox.Max.Y = p.Y
	}
	if p.Z > bbox.Max.Z {
		bbox.Max.Z = p.Z
	}
}

func (bbox *BoundingBox3D) Contains(p Point3D) bool {
	return (bbox.Min.X <= p.X && p.X <= bbox.Max.X &&
		bbox.Min.Y <= p.Y && p.Y <= bbox.Max.Y &&
		bbox.Min.Z <= p.Z && p.Z <= bbox.Max.Z)
}

func (bbox *BoundingBox3D) Translate(p Point3D) {
	bbox.Min.Translate(p)
	bbox.Max.Translate(p)
}

func (bbox *BoundingBox3D) GetVolume() *big.Int {
	minX := big.NewInt(int64(bbox.Min.X))
	maxX := big.NewInt(int64(bbox.Max.X))
	minY := big.NewInt(int64(bbox.Min.Y))
	maxY := big.NewInt(int64(bbox.Max.Y))
	minZ := big.NewInt(int64(bbox.Min.Z))
	maxZ := big.NewInt(int64(bbox.Max.Z))
	vol := big.NewInt(1)
	vol.Mul(maxX.Sub(maxX, minX), maxY.Sub(maxY, minY))
	vol.Mul(vol, maxZ.Sub(maxZ, minZ))
	return vol
}

func (bbox *BoundingBox3D) GetCorners() [8]Point3D {
	corners := [8]Point3D{}
	for i := range corners {
		newCorner := bbox.Min
		if i%2 == 1 {
			newCorner.Translate(Point3D{bbox.Max.X - bbox.Min.X, 0, 0})
		}
		if (i/2)%2 == 1 {
			newCorner.Translate(Point3D{0, bbox.Max.Y - bbox.Min.Y, 0})
		}
		if (i/4)%2 == 1 {
			newCorner.Translate(Point3D{0, 0, bbox.Max.Z - bbox.Min.Z})
		}
		corners[i] = newCorner
	}
	return corners
}
