package main

import "fmt"

// This is a translation of the day 21 input assembly program into Go.
// This native version runs much quicker than the emulator.

func main() {
	registers := [6]int{}
	register2Values := make(map[int]int)
	for i := 0; true; i++ {
		registers[5] = registers[2] | 65536
		registers[2] = 2238642
		for {
			registers[2] += registers[5] & 255
			registers[2] = ((registers[2] & 16777215) * 65899) & 16777215
			if 256 > registers[5] {
				break
			}
			registers[3] = 0
			for {
				registers[1] = (registers[3] + 1) * 256
				if registers[1] > registers[5] {
					break
				}
				registers[3] += 1
			}
			registers[5] = registers[3]
		}
		if _, present := register2Values[registers[2]]; present {
			break
		}
		register2Values[registers[2]] = i
		if len(register2Values)%1000 == 0 {
			fmt.Println(len(register2Values))
		}
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
