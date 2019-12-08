package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/assembly"
	"github.com/rgambee/aoc2018/utils"
	"regexp"
)

const (
	NUM_REGISTERS          = 6
	EXIT_CHECK_INSTRUCTION = 28
)

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	instructionRE := regexp.MustCompile("([a-z]{4}) ([-]?[0-9]+) ([0-9]+) ([0-9]+)")
	ipBindRE := regexp.MustCompile("#ip ([0-9])")

	ipRegister := -1
	instructions := make([]assembly.Instruction, 0)
	// Read program
	for scanner.Scan() {
		newLine := scanner.Text()
		// Assume this is a normal instruction
		instrStr, err := utils.ParseString(newLine, instructionRE, 4)
		if err != nil {
			// Try interpreting it as an IP bind command
			ipBind, err := utils.ParseString(newLine, ipBindRE, 1)
			if err != nil {
				panic(err)
			}
			ipRegister = utils.StringToInt(ipBind[0])
			continue
		}
		abcStr := instrStr[1:]
		abcInt := *utils.StringSliceToIntSlice(&abcStr)
		instructions = append(instructions, assembly.Instruction{
			instrStr[0], abcInt[0], abcInt[1], abcInt[2]})
	}

	registers := make(assembly.RegisterSet, NUM_REGISTERS)
	// Set registers[0] to -1 so it'll never exit
	registers[0] = -1
	register2Values := make(map[int]int)
	for ip, iter := 0, 0; ip >= 0 && ip < len(instructions); ip++ {
		if ipRegister >= 0 {
			// Write instruction pointer to bound register
			registers[ipRegister] = ip
		}
		if ip == EXIT_CHECK_INSTRUCTION {
			if _, present := register2Values[registers[2]]; present {
				// Program has started repeating, so we can break
				break
			}
			register2Values[registers[2]] = iter
			if len(register2Values)%1000 == 0 {
				fmt.Println(len(register2Values))
			}
		}
		// Execute instruction
		instruction := instructions[ip]
		registers = assembly.ALL_OPERATORS[instruction.OpName].Operate(
			registers, instruction.A, instruction.B, instruction.C)
		if ipRegister >= 0 {
			// Read instruction pointer from bound register
			ip = registers[ipRegister]
		}
		iter++
	}

	fastest, slowest := -1, -1
	for reg2, iter := range register2Values {
		if fastest < 0 || iter < register2Values[fastest] {
			fastest = reg2
		}
		if slowest < 0 || iter > register2Values[slowest] {
			slowest = reg2
		}
	}

	fmt.Println("PART 1")
	fmt.Println("Value of register 0 to exit fastest:", fastest)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Value of register 0 to exit slowest:", slowest)
}
