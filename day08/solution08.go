package main

import (
	"bufio"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
)

type Node struct {
	children []*Node
	metadata []int
}

func getNextToken(scanner *bufio.Scanner) int {
	if !scanner.Scan() {
		log.Panic("Ran out of input")
	}
	return utils.StringToInt(scanner.Text())
}

func newNode(scanner *bufio.Scanner) *Node {
	numChildren := getNextToken(scanner)
	numMetadata := getNextToken(scanner)
	if numMetadata < 1 {
		log.Panic("Number of metadata entries cannot be less than one: ", numMetadata)
	}
	children := make([]*Node, numChildren)
	metadata := make([]int, numMetadata)
	for i := range children {
		children[i] = newNode(scanner)
	}
	for i := range metadata {
		metadata[i] = getNextToken(scanner)
	}
	node := Node{children, metadata}
	return &node
}

func sumMetadataEntries(node *Node) int {
	total := utils.SumSlice(&node.metadata)
	for _, c := range node.children {
		total += sumMetadataEntries(c)
	}
	return total
}

func getNodeValue(node *Node) int {
	if len(node.children) == 0 {
		return sumMetadataEntries(node)
	} else {
		value := 0
		for _, m := range node.metadata {
			if 0 <= m-1 && m-1 < len(node.children) {
				value += getNodeValue(node.children[m-1])
			}
		}
		return value
	}
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetWordScanner(file)
	root := newNode(scanner)

	// Add up all metadata entries
	sumOfMetadataEntries := sumMetadataEntries(root)

	// Find value of root node
	rootValue := getNodeValue(root)

	fmt.Println("PART 1")
	fmt.Println("Total number of metadata entries is", sumOfMetadataEntries)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Value of root node", rootValue)
}
