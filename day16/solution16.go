package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"regexp"
)

type RegisterSet [4]int
type Instruction [4]int

type Operator interface {
	Operate(rs RegisterSet, a, b, c int) RegisterSet
	String() string
}

type ADDR_Struct struct{ name string }
type ADDI_Struct struct{ name string }
type MULR_Struct struct{ name string }
type MULI_Struct struct{ name string }
type BANR_Struct struct{ name string }
type BANI_Struct struct{ name string }
type BORR_Struct struct{ name string }
type BORI_Struct struct{ name string }
type SETR_Struct struct{ name string }
type SETI_Struct struct{ name string }
type GTIR_Struct struct{ name string }
type GTRI_Struct struct{ name string }
type GTRR_Struct struct{ name string }
type EQIR_Struct struct{ name string }
type EQRI_Struct struct{ name string }
type EQRR_Struct struct{ name string }

func (s ADDR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] + rs[b]
	return rs
}
func (s ADDR_Struct) String() string {
	return s.name
}

func (s ADDI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] + b
	return rs
}
func (s ADDI_Struct) String() string {
	return s.name
}

func (s MULR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] * rs[b]
	return rs
}
func (s MULR_Struct) String() string {
	return s.name
}

func (s MULI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] * b
	return rs
}
func (s MULI_Struct) String() string {
	return s.name
}

func (s BANR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] & rs[b]
	return rs
}
func (s BANR_Struct) String() string {
	return s.name
}

func (s BANI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] & b
	return rs
}
func (s BANI_Struct) String() string {
	return s.name
}

func (s BORR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] | rs[b]
	return rs
}
func (s BORR_Struct) String() string {
	return s.name
}

func (s BORI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a] | b
	return rs
}
func (s BORI_Struct) String() string {
	return s.name
}

func (s SETR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = rs[a]
	return rs
}
func (s SETR_Struct) String() string {
	return s.name
}

func (s SETI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = a
	return rs
}
func (s SETI_Struct) String() string {
	return s.name
}

func (s GTIR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(a > rs[b])
	return rs
}
func (s GTIR_Struct) String() string {
	return s.name
}

func (s GTRI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] > b)
	return rs
}
func (s GTRI_Struct) String() string {
	return s.name
}

func (s GTRR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] > rs[b])
	return rs
}
func (s GTRR_Struct) String() string {
	return s.name
}

func (s EQIR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(a == rs[b])
	return rs
}
func (s EQIR_Struct) String() string {
	return s.name
}

func (s EQRI_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] == b)
	return rs
}
func (s EQRI_Struct) String() string {
	return s.name
}

func (s EQRR_Struct) Operate(rs RegisterSet, a, b, c int) RegisterSet {
	rs[c] = bool2int(rs[a] == rs[b])
	return rs
}
func (s EQRR_Struct) String() string {
	return s.name
}

var ADDR_ID = &ADDR_Struct{"addr"}
var ADDI_ID = &ADDI_Struct{"addi"}
var MULR_ID = &MULR_Struct{"mulr"}
var MULI_ID = &MULI_Struct{"muli"}
var BANR_ID = &BANR_Struct{"banr"}
var BANI_ID = &BANI_Struct{"bani"}
var BORR_ID = &BORR_Struct{"borr"}
var BORI_ID = &BORI_Struct{"bori"}
var SETR_ID = &SETR_Struct{"setr"}
var SETI_ID = &SETI_Struct{"seti"}
var GTIR_ID = &GTIR_Struct{"gtir"}
var GTRI_ID = &GTRI_Struct{"gtri"}
var GTRR_ID = &GTRR_Struct{"gtrr"}
var EQIR_ID = &EQIR_Struct{"eqir"}
var EQRI_ID = &EQRI_Struct{"eqri"}
var EQRR_ID = &EQRR_Struct{"eqrr"}

var ALL_OPERATORS [16]Operator = [16]Operator{
	ADDR_ID, ADDI_ID, MULR_ID, MULI_ID, BANR_ID, BANI_ID, BORR_ID, BORI_ID,
	SETR_ID, SETI_ID, GTIR_ID, GTRI_ID, GTRR_ID, EQIR_ID, EQRI_ID, EQRR_ID}

func bool2int(b bool) int {
	if b {
		return 1
	}
	return 0
}

func getMatchingOperators(instr Instruction, before, after RegisterSet) []Operator {
	a, b, c := instr[1], instr[2], instr[3]
	matchingOperators := make([]Operator, 0)
	for _, oper := range ALL_OPERATORS {
		if oper.Operate(before, a, b, c) == after {
			matchingOperators = append(matchingOperators, oper)
		}
	}
	return matchingOperators
}

func operatorPresent(operators []Operator, oper Operator) bool {
	present := false
	for _, o := range operators {
		if oper == o {
			present = true
			break
		}
	}
	return present
}

func removeOperator(operators []Operator, operToRemove Operator) []Operator {
	result := make([]Operator, 0)
	for _, oper := range operators {
		if oper != operToRemove {
			result = append(result, oper)
		}
	}
	return result
}

func union(operators1, operators2 []Operator) []Operator {
	// Return slice of Operators that are present in both input slices
	commonOperators := make([]Operator, 0)
	for _, oper2 := range operators2 {
		if operatorPresent(operators1, oper2) {
			commonOperators = append(commonOperators, oper2)
		}
	}
	return commonOperators
}

func printDecodedOpcodes(decodedOpcodes *[][]Operator) {
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
	decodedOpcodes := make([][]Operator, len(ALL_OPERATORS))
	for i := range decodedOpcodes {
		decodedOpcodes[i] = ALL_OPERATORS[:]
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

		var before, after RegisterSet
		var instr Instruction
		copy(before[:], *utils.StringSliceToIntSlice(&beforeStr))
		copy(after[:], *utils.StringSliceToIntSlice(&afterStr))
		copy(instr[:], *utils.StringSliceToIntSlice(&instrStr))

		opcode := instr[0]
		matchingOperators := getMatchingOperators(instr, before, after)
		if len(matchingOperators) >= 3 {
			samplesWith3OrMoreMatches++
		}
		// Trim out non-matching operators
		// TODO: check whether opcode matches operation that no other opcodes do
		decodedOpcodes[opcode] = union(decodedOpcodes[opcode], matchingOperators)
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
	printDecodedOpcodes(&decodedOpcodes)

	// Read program
	registers := RegisterSet{}
	for scanner.Scan() {
		if scanner.Text() == "" {
			// Skip over blank lines
			continue
		}
		instrStr, err := utils.ParseString(scanner.Text(), instructionRE, 4)
		if err != nil {
			panic(err)
		}
		var instr Instruction
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
