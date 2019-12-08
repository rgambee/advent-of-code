package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/assembly"
	"github.com/rgambee/aoc2018/utils"
	"regexp"
)

const NUM_REGISTERS = 6

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	instructionRE := regexp.MustCompile("([a-z]{4}) ([0-9]+) ([0-9]+) ([0-9]+)")
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

	// Execute program with all registers initialized to 0
	registers := make(assembly.RegisterSet, NUM_REGISTERS)
	for ip := 0; ip >= 0 && ip < len(instructions); ip++ {
		if ipRegister >= 0 {
			// Write instruction pointer to bound register
			registers[ipRegister] = ip
		}
		// Execute instruction
		instruction := instructions[ip]
		registers = assembly.ALL_OPERATORS[instruction.OpName].Operate(
			registers, instruction.A, instruction.B, instruction.C)
		if ipRegister >= 0 {
			// Read instruction pointer from bound register
			ip = registers[ipRegister]
		}
		// fmt.Println(registers)
	}
	fmt.Println("PART 1")
	fmt.Println("Final value in register 0 is", registers[0])
	fmt.Println()

	// Execute program with R0 initialized to 1
	registers = make(assembly.RegisterSet, NUM_REGISTERS)
	registers[0] = 1
	for ip, i := 0, 0; ip >= 0 && ip < len(instructions); ip++ {
		if ipRegister >= 0 {
			// Write instruction pointer to bound register
			registers[ipRegister] = ip
		}
		// Execute instruction
		instruction := instructions[ip]
		registers = assembly.ALL_OPERATORS[instruction.OpName].Operate(
			registers, instruction.A, instruction.B, instruction.C)
		if ipRegister >= 0 {
			// Read instruction pointer from bound register
			ip = registers[ipRegister]
		}
		if ip == 33 {
			// At this instruction, registers[5] reaches its final value
			break
		}
		i++
	}
	// Add up factors of registers[5]
	sumOfFactors := 0
	for i := 1; i <= registers[5]; i++ {
		if registers[5]%i == 0 {
			sumOfFactors += i
		}
	}
	fmt.Println("PART 2")
	fmt.Println("Final value in register 0 is", sumOfFactors)
}
