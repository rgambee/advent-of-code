package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
)

type Node struct {
	name     string
	children []*Node
}

func (n *Node) find(s string) *Node {
	if n.name == s {
		return n
	} else if n.children == nil {
		return nil
	} else {
		for _, child := range n.children {
			childFind := child.find(s)
			if childFind != nil {
				return childFind
			}
		}
		return nil
	}
}

func (n *Node) addChild(child *Node) {
	if child == nil {
		return
	}
	// Insert child in alphabetical order
	for i, c := range n.children {
		if c.name == child.name {
			return
		} else if c.name > child.name {
			n.children = append(n.children, nil)
			copy(n.children[i+1:], n.children[i:])
			n.children[i] = child
			return
		}
	}
	n.children = append(n.children, child)
}

func (n *Node) deleteChild(child *Node) {
	if child == nil {
		return
	}
	for i, c := range n.children {
		if c == child {
			n.children = append(n.children[:i], n.children[i+1:]...)
			for _, cc := range child.children {
				n.addChild(cc)
			}
			return
		}
	}
	log.Panic("Node", n, "doesn't have", child, "as direct child")
}

func (n *Node) copy() *Node {
	newNode := Node{n.name, make([]*Node, len(n.children))}
	copy(newNode.children, n.children)
	return &newNode
}

func getTimeForTask(taskName string) int {
	if len(taskName) != 1 {
		log.Panic("Invalid task name", taskName)
	}
	// 'A' == 1, 'B' == 2, 'C' == 3, etc.
	taskNum := int(taskName[0] - 'A' + 1)
	return taskNum + 60
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	re := regexp.MustCompile("Step ([A-Z]) .* before step ([A-Z]) .*")

	root := &Node{"", make([]*Node, 0)}
	for scanner.Scan() {
		newLine := scanner.Text()
		parsedLine := utils.ParseString(newLine, re, 2)
		earlierStep, laterStep := parsedLine[0], parsedLine[1]
		// Need to update root such that earlierStep comes before laterStep
		earlierNode := root.find(earlierStep)
		laterNode := root.find(laterStep)
		if earlierNode == nil {
			earlierNode = &Node{earlierStep, make([]*Node, 0)}
			root.addChild(earlierNode)
		}
		if laterNode == nil {
			laterNode = &Node{laterStep, make([]*Node, 0)}
		}
		if earlierNode.find(laterStep) == nil {
			earlierNode.addChild(laterNode)
		}
		if earlierNode.find(laterStep) == nil {
			log.Panic("Earlier step does not come before later step")
		}
	}
	rootCopy := root.copy()

	// Iterate over root
	orderedSteps := make([]string, 0)
	for len(root.children) > 0 {
		for i := 0; i < len(root.children); {
			candidateStep := root.children[i].name
			// Check whether any other steps are a prerequisite for candidateStep
			safeToRemove := true
			for j, c := range root.children {
				if i == j {
					continue
				}
				if c.find(candidateStep) != nil {
					safeToRemove = false
					break
				}
			}
			if safeToRemove {
				orderedSteps = append(orderedSteps, candidateStep)
				root.deleteChild(root.children[i])
				i = 0
			} else {
				i++
			}
		}
	}

	// Find time to complete tasks
	root = rootCopy.copy()
	seconds := 0
	activeSteps := make(map[*Node]int)
	for len(root.children) > 0 {
		for i := 0; i < len(root.children); i++ {
			candidateStep := root.children[i].name
			// Check whether any other steps are a prerequisite for candidateStep
			safeToRemove := true
			for j, c := range root.children {
				if i == j {
					continue
				}
				if c.find(candidateStep) != nil {
					safeToRemove = false
					break
				}
			}
			if safeToRemove {
				if _, present := activeSteps[root.children[i]]; !present {
					// Make this step active
					activeSteps[root.children[i]] = getTimeForTask(candidateStep)
				}
			}
		}
		for node := range activeSteps {
			activeSteps[node]--
			if activeSteps[node] <= 0 {
				root.deleteChild(node)
				delete(activeSteps, node)
			}
		}
		seconds++
	}

	fmt.Println("PART 1")
	fmt.Printf("Instruction order is ")
	for _, step := range orderedSteps {
		fmt.Printf(string(step))
	}
	fmt.Println()
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Printf("Total time to complete is %v seconds\n", seconds)
}
