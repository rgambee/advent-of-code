package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"math/big"
	"regexp"
)

type Octree struct {
	bbox          utils.BoundingBox3D
	children      [8]*Octree
	containedBots []*Bot
}

func (ot *Octree) contains(bot *Bot) bool {
	bboxContainsCornerOfSphere := (ot.bbox.Contains(bot.position) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X + bot.sigRadius, bot.position.Y, bot.position.Z}) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X - bot.sigRadius, bot.position.Y, bot.position.Z}) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X, bot.position.Y + bot.sigRadius, bot.position.Z}) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X, bot.position.Y - bot.sigRadius, bot.position.Z}) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X, bot.position.Y, bot.position.Z + bot.sigRadius}) ||
		ot.bbox.Contains(utils.Point3D{
			bot.position.X, bot.position.Y, bot.position.Z - bot.sigRadius}))
	sphereContainsCornerOfBBox := false
	for _, corner := range ot.bbox.GetCorners() {
		sphereContainsCornerOfBBox = sphereContainsCornerOfBBox || bot.isInRange(corner)
	}
	return bboxContainsCornerOfSphere || sphereContainsCornerOfBBox
}

func (ot *Octree) addBotIfContained(bot *Bot) {
	if ot.contains(bot) {
		ot.containedBots = append(ot.containedBots, bot)
	}
}

func (ot *Octree) countContainedBots() int {
	return len(ot.containedBots)
}

func (ot *Octree) isLeaf() bool {
	anyNil := false
	allNil := true
	for _, child := range ot.children {
		anyNil = anyNil || (child == nil)
		allNil = allNil && (child == nil)
	}
	if anyNil != allNil {
		log.Panic("Some children are nil but others aren't: ", ot)
	}
	return allNil
}

func (ot *Octree) createChildren() {
	// TODO: don't double-count midpoint for octrees with odd side lengths
	midPoint := utils.Point3D{
		(ot.bbox.Min.X + ot.bbox.Max.X) / 2,
		(ot.bbox.Min.Y + ot.bbox.Max.Y) / 2,
		(ot.bbox.Min.Z + ot.bbox.Max.Z) / 2}
	for i := 0; i < 8; i++ {
		subBBox := utils.BoundingBox3D{ot.bbox.Min, midPoint}
		if i%2 == 1 {
			subBBox.Translate(utils.Point3D{ot.bbox.Max.X - midPoint.X, 0, 0})
		}
		if (i/2)%2 == 1 {
			subBBox.Translate(utils.Point3D{0, ot.bbox.Max.Y - midPoint.Y, 0})
		}
		if (i/4)%2 == 1 {
			subBBox.Translate(utils.Point3D{0, 0, ot.bbox.Max.Z - midPoint.Z})
		}
		ot.children[i] = &Octree{bbox: subBBox}
		for _, bot := range ot.containedBots {
			ot.children[i].addBotIfContained(bot)
		}
	}
}

func (ot *Octree) findMostPopulatedLeaf() *Octree {
	if ot.isLeaf() {
		return ot
	}
	var mostPopulatedLeaf *Octree
	mostBots := -1
	for _, child := range ot.children {
		mostPopulatedChildLeaf := child.findMostPopulatedLeaf()
		mostChildBots := mostPopulatedChildLeaf.countContainedBots()
		if mostChildBots > mostBots {
			mostBots = mostChildBots
			mostPopulatedLeaf = mostPopulatedChildLeaf
		} else if mostChildBots == mostBots {
			currCornerDists := make([]int, 8)
			for i, corner := range mostPopulatedLeaf.bbox.GetCorners() {
				currCornerDists[i] = corner.DistanceTo(utils.Point3D{0, 0, 0})
			}
			childCornerDists := make([]int, 8)
			for i, corner := range mostPopulatedChildLeaf.bbox.GetCorners() {
				childCornerDists[i] = corner.DistanceTo(utils.Point3D{0, 0, 0})
			}
			if utils.FindMin(childCornerDists...) < utils.FindMin(currCornerDists...) {
				mostPopulatedLeaf = mostPopulatedChildLeaf
			}
		}
	}
	if mostPopulatedLeaf == nil {
		log.Panic("Couldn't find most populated leaf of", ot)
	}
	return mostPopulatedLeaf
}

func (ot *Octree) findBestPosition() utils.Point3D {
	// Exhaustively search this octree's bounding box to find the
	// position in range of most bots. Only sensible for small octrees.
	mostBots := -1
	bestPos := ot.bbox.Min
	for x := ot.bbox.Min.X; x <= ot.bbox.Max.X; x++ {
		for y := ot.bbox.Min.Y; y <= ot.bbox.Max.Y; y++ {
			for z := ot.bbox.Min.Z; z <= ot.bbox.Max.Z; z++ {
				newPos := utils.Point3D{x, y, z}
				botsInRange := countBotsInRange(&ot.containedBots, newPos)
				if botsInRange > mostBots {
					mostBots = botsInRange
					bestPos = newPos
				} else if botsInRange == mostBots {
					if newPos.DistanceTo(utils.Point3D{0, 0, 0}) < bestPos.DistanceTo(utils.Point3D{0, 0, 0}) {
						bestPos = newPos
					}
				}
			}
		}
	}
	if mostBots < 0 {
		log.Panic("Couldn't iterate over bounding box ", ot.bbox)
	}
	return bestPos
}

type Bot struct {
	position  utils.Point3D
	sigRadius int
}

func (b *Bot) isInRange(pos utils.Point3D) bool {
	dist := b.position.DistanceTo(pos)
	return dist <= b.sigRadius
}

func countBotsInRange(bots *[]*Bot, pos utils.Point3D) int {
	total := 0
	for _, bot := range *bots {
		if bot.isInRange(pos) {
			total++
		}
	}
	return total
}

func findPositionInRangeOfMostBots(octree *Octree) utils.Point3D {
	currentOctree := octree
	for currentOctree.bbox.GetVolume().Cmp(big.NewInt(15)) == 1 {
		currentOctree = octree.findMostPopulatedLeaf()
		currentOctree.createChildren()
	}
	bestPos := currentOctree.findBestPosition()
	return bestPos
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	botRE := regexp.MustCompile("pos=<([-]?\\d+),([-]?\\d+),([-]?\\d+)>, r=(\\d+)")
	bots := make([]*Bot, 0)
	bbox := utils.BoundingBox3D{}
	mostPowerfulBot := -1
	for scanner.Scan() {
		parsedLine_str, err := utils.ParseString(scanner.Text(), botRE, 4)
		if err != nil {
			panic(err)
		}
		parsedLine_int := *utils.StringSliceToIntSlice(&parsedLine_str)
		newBot := Bot{
			utils.Point3D{parsedLine_int[0], parsedLine_int[1], parsedLine_int[2]},
			parsedLine_int[3]}
		bots = append(bots, &newBot)
		if mostPowerfulBot < 0 || bots[mostPowerfulBot].sigRadius < newBot.sigRadius {
			mostPowerfulBot = len(bots) - 1
		}
		bbox.Update(newBot.position)
	}

	numBotsInRangeOfMostPowerfulBot := 0
	for _, bot := range bots {
		if bots[mostPowerfulBot].isInRange(bot.position) {
			numBotsInRangeOfMostPowerfulBot++
		}
	}

	// Expand the bounding box to be a cube for the octree
	maxSideLength := utils.FindMax(
		bbox.Max.X-bbox.Min.X, bbox.Max.Y-bbox.Min.Y, bbox.Max.Z-bbox.Min.Z)
	octreeBBoxMin := bbox.Min
	octreeBBoxMax := bbox.Min
	octreeBBoxMax.Translate(utils.Point3D{maxSideLength, maxSideLength, maxSideLength})
	octreeBBox := utils.BoundingBox3D{octreeBBoxMin, octreeBBoxMax}
	octree := Octree{bbox: octreeBBox, containedBots: bots}
	bestPosition := findPositionInRangeOfMostBots(&octree)
	// fmt.Println("Best position is", bestPosition)
	distanceToBestPos := bestPosition.DistanceTo(utils.Point3D{0, 0, 0})

	fmt.Println("PART 1")
	fmt.Println("Number of bots in range of most powerful one:",
		numBotsInRangeOfMostPowerfulBot)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Distance to position in range of most bots:",
		distanceToBestPos)
}
