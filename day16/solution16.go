package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/assembly"
	"github.com/rgambee/aoc2018/utils"
	"regexp"
)

type CodedInstruction [4]int

const NUM_REGISTERS = 4

func getMatchingOperators(instr CodedInstruction,
	before, after assembly.RegisterSet) []assembly.Operator {
	a, b, c := instr[1], instr[2], instr[3]
	beforeCopy := make(assembly.RegisterSet, len(before))
	matchingOperators := make([]assembly.Operator, 0)
	for _, oper := range assembly.ALL_OPERATORS {
		copy(beforeCopy, before)
		match := true
		result := oper.Operate(beforeCopy, a, b, c)
		for i, reg := range result {
			if reg != after[i] {
				match = false
				break
			}
		}
		if match {
			matchingOperators = append(matchingOperators, oper)
		}
	}
	return matchingOperators
}

func operatorPresent(operators []assembly.Operator, oper assembly.Operator) bool {
	for _, o := range operators {
		if oper == o {
			return true
		}
	}
	return false
}

func removeOperator(operators []assembly.Operator,
	operToRemove assembly.Operator) []assembly.Operator {
	result := make([]assembly.Operator, 0)
	for _, oper := range operators {
		if oper != operToRemove {
			result = append(result, oper)
		}
	}
	return result
}

func intersection(operators1, operators2 []assembly.Operator) []assembly.Operator {
	// Return slice of Operators that are present in both input slices
	commonOperators := make([]assembly.Operator, 0)
	for _, oper2 := range operators2 {
		if operatorPresent(operators1, oper2) {
			commonOperators = append(commonOperators, oper2)
		}
	}
	return commonOperators
}

func printDecodedOpcodes(decodedOpcodes *[][]assembly.Operator) {
	for opcode, operations := range *decodedOpcodes {
		fmt.Printf("%v: ", opcode)
		for _, oper := range operations {
			fmt.Printf("%v ", oper)
		}
		fmt.Println()
	}
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	beforeRE := regexp.MustCompile("Before: \\[(\\d+), (\\d+), (\\d+), (\\d+)\\]")
	instructionRE := regexp.MustCompile("(\\d+) (\\d+) (\\d+) (\\d+)")
	afterRE := regexp.MustCompile("After:  \\[(\\d+), (\\d+), (\\d+), (\\d+)\\]")

	samplesWith3OrMoreMatches := 0
	decodedOpcodes := make([][]assembly.Operator, len(assembly.ALL_OPERATORS))
	for i := range decodedOpcodes {
		operSlice := make([]assembly.Operator, len(assembly.ALL_OPERATORS))
		j := 0
		for _, oper := range assembly.ALL_OPERATORS {
			operSlice[j] = oper
			j++
		}
		decodedOpcodes[i] = operSlice
	}

	// Read in samples and determine opcodes
	for {
		scanner.Scan()
		beforeStr, err := utils.ParseString(scanner.Text(), beforeRE, 4)
		if err != nil {
			// We've reached the end of the samples, so exit the loop
			break
		}
		scanner.Scan()
		instrStr, err := utils.ParseString(scanner.Text(), instructionRE, 4)
		if err != nil {
			panic(err)
		}
		scanner.Scan()
		afterStr, err := utils.ParseString(scanner.Text(), afterRE, 4)
		if err != nil {
			panic(err)
		}
		// Skip empty line between samples
		scanner.Scan()

		before := make(assembly.RegisterSet, len(beforeStr))
		after := make(assembly.RegisterSet, len(afterStr))
		var instr CodedInstruction
		copy(before, *utils.StringSliceToIntSlice(&beforeStr))
		copy(after, *utils.StringSliceToIntSlice(&afterStr))
		copy(instr[:], *utils.StringSliceToIntSlice(&instrStr))

		opcode := instr[0]
		matchingOperators := getMatchingOperators(instr, before, after)
		if len(matchingOperators) >= 3 {
			samplesWith3OrMoreMatches++
		}
		// Trim out non-matching operators
		// TODO: check whether opcode matches operation that no other opcodes do
		decodedOpcodes[opcode] = intersection(decodedOpcodes[opcode], matchingOperators)
		// If this opcode has been fully defined, remove the corresponding
		// operation from all other opcodes
		if len(decodedOpcodes[opcode]) == 1 {
			for oc, operations := range decodedOpcodes {
				if oc != opcode {
					decodedOpcodes[oc] = removeOperator(
						operations, decodedOpcodes[opcode][0])
				}
			}
		}
	}
	// Make sure each opcode decodes to exactly one operator
	for opcode, operator := range decodedOpcodes {
		if len(operator) != 1 {
			fmt.Printf("Couldn't find exactly 1 match for opcode %v: %v\n",
				opcode, operator)
		}
	}
	// printDecodedOpcodes(&decodedOpcodes)

	// Read program
	registers := make(assembly.RegisterSet, NUM_REGISTERS)
	for scanner.Scan() {
		if scanner.Text() == "" {
			// Skip over blank lines
			continue
		}
		instrStr, err := utils.ParseString(scanner.Text(), instructionRE, 4)
		if err != nil {
			panic(err)
		}
		var instr CodedInstruction
		copy(instr[:], *utils.StringSliceToIntSlice(&instrStr))
		opcode, a, b, c := instr[0], instr[1], instr[2], instr[3]
		registers = decodedOpcodes[opcode][0].Operate(registers, a, b, c)
	}

	fmt.Println("PART 1")
	fmt.Println("Samples with 3 or more matching opcodes:",
		samplesWith3OrMoreMatches)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Final value of R0:", registers[0])
}
