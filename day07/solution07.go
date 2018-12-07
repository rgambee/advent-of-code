package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
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

func parseLine(line string, re *regexp.Regexp) (string, string) {
	matches := re.FindStringSubmatch(line)
	if matches == nil {
		log.Fatal("No matches found for %v", line)
	}
	if len(matches) != 3 {
		log.Fatal("Expected 3 matches but found %v", matches)
	}
	return matches[1], matches[2]
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

	re := regexp.MustCompile("Step ([A-Z]) .* before step ([A-Z]) .*")

	reader := bufio.NewReader(file)
	scanner := bufio.NewScanner(reader)

	tree := &Node{"", make([]*Node, 0)}
	for scanner.Scan() {
		newLine := scanner.Text()
		earlierStep, laterStep := parseLine(newLine, re)
		// Need to update tree such that earlierStep comes before laterStep
		earlierNode := tree.find(earlierStep)
		laterNode := tree.find(laterStep)
		if earlierNode == nil {
			earlierNode = &Node{earlierStep, make([]*Node, 0)}
			tree.addChild(earlierNode)
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
	treeCopy := tree.copy()

	// Iterate over tree
	orderedSteps := make([]string, 0)
	for len(tree.children) > 0 {
		for i := 0; i < len(tree.children); {
			candidateStep := tree.children[i].name
			// Check whether any other steps are a prerequisite for candidateStep
			safeToRemove := true
			for j, c := range tree.children {
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
				tree.deleteChild(tree.children[i])
				i = 0
			} else {
				i++
			}
		}
	}

	// Find time to complete tasks
	tree = treeCopy.copy()
	seconds := 0
	activeSteps := make(map[*Node]int)
	for len(tree.children) > 0 {
		for i := 0; i < len(tree.children); i++ {
			candidateStep := tree.children[i].name
			// Check whether any other steps are a prerequisite for candidateStep
			safeToRemove := true
			for j, c := range tree.children {
				if i == j {
					continue
				}
				if c.find(candidateStep) != nil {
					safeToRemove = false
					break
				}
			}
			if safeToRemove {
				if _, present := activeSteps[tree.children[i]]; !present {
					// Make this step active
					activeSteps[tree.children[i]] = getTimeForTask(candidateStep)
				}
			}
		}
		for node := range activeSteps {
			activeSteps[node]--
			if activeSteps[node] <= 0 {
				tree.deleteChild(node)
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
