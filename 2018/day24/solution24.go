package main

import (
	"bufio"
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
	"sort"
	"strings"
)

type DamageType string

type Group struct {
	units, hitpoints, attack, initiative int
	attackType                           DamageType
	weaknesses, immunities               []DamageType
}

func newGroup(description []string) *Group {
	units := utils.StringToInt(description[0])
	hitpoints := utils.StringToInt(description[1])
	weaknesses := getWeaknesses(description[2])
	immunities := getImmunities(description[2])
	attack := utils.StringToInt(description[3])
	attackType := DamageType(description[4])
	initiative := utils.StringToInt(description[5])
	group := Group{
		units, hitpoints, attack, initiative,
		attackType, weaknesses, immunities}
	return &group
}

func (g *Group) copy() *Group {
	groupCopy := *g
	return &groupCopy
}

func (g *Group) getEffectivePower() int {
	return g.units * g.attack
}

func (g *Group) isImmuneTo(dt DamageType) bool {
	for _, imm := range g.immunities {
		if imm == dt {
			return true
		}
	}
	return false
}

func (g *Group) isWeakTo(dt DamageType) bool {
	for _, weak := range g.weaknesses {
		if weak == dt {
			return true
		}
	}
	return false
}

func (g *Group) getAttackDamage(defender *Group) int {
	rawDamage := g.getEffectivePower()
	if defender.isImmuneTo(g.attackType) {
		return 0
	} else if defender.isWeakTo(g.attackType) {
		return rawDamage * 2
	}
	return rawDamage
}

func (g *Group) takeDamage(adjustedDamage int) {
	// fmt.Printf("Losing %v units\n", adjustedDamage/g.hitpoints)
	g.units -= adjustedDamage / g.hitpoints
}

func (g *Group) isAlive() bool {
	return g.units > 0
}

func (attacker *Group) attackTarget(defender *Group) {
	// fmt.Printf("Applying %v damage\n", attacker.getAttackDamage(defender))
	defender.takeDamage(attacker.getAttackDamage(defender))
}

func (attacker *Group) selectTarget(defendingGroups *[]*Group) *Group {
	selectedGroup := -1
	mostDamage := 0
	for i, defender := range *defendingGroups {
		potentialDamage := attacker.getAttackDamage(defender)
		if potentialDamage > mostDamage {
			mostDamage = potentialDamage
			selectedGroup = i
		} else if potentialDamage == mostDamage && potentialDamage > 0 {
			currPow := defender.getEffectivePower()
			selPow := (*defendingGroups)[selectedGroup].getEffectivePower()
			currInit := defender.initiative
			selInit := (*defendingGroups)[selectedGroup].initiative
			if currPow > selPow || (currPow == selPow && currInit > selInit) {
				selectedGroup = i
			}
		}
	}
	if mostDamage == 0 && selectedGroup == -1 {
		return nil
	} else if mostDamage > 0 && selectedGroup >= 0 {
		targetGroup := (*defendingGroups)[selectedGroup]
		// Remove selected defending group from list of potential targets
		*defendingGroups = append(
			(*defendingGroups)[:selectedGroup], (*defendingGroups)[selectedGroup+1:]...)
		return targetGroup
	}
	log.Panic("Couldn't pick target")
	return nil
}

func copyGroupsShallow(groups *[]*Group) []*Group {
	// Creates a new slice of pointers but does not copy underlying Groups objects
	sliceCopy := make([]*Group, len(*groups))
	copy(sliceCopy, *groups)
	return sliceCopy
}

func copyGroupsDeep(groups *[]*Group) []*Group {
	// Creates full copy of underlying Groups objects
	sliceCopy := make([]*Group, len(*groups))
	for i, group := range *groups {
		sliceCopy[i] = group.copy()
	}
	return sliceCopy
}

func boostAttack(groups *[]*Group, attackBoost int) {
	for _, group := range *groups {
		group.attack += attackBoost
	}
}

func pickTargets(attackingGroups *[]*Group, defendingGroups *[]*Group) map[*Group]*Group {
	attackingCopy := copyGroupsShallow(attackingGroups)
	defendingCopy := copyGroupsShallow(defendingGroups)
	targets := make(map[*Group]*Group)
	for len(attackingCopy) > 0 {
		selectedGroup := -1
		mostPower := 0
		for i, attacker := range attackingCopy {
			if attacker.getEffectivePower() > mostPower {
				mostPower = attacker.getEffectivePower()
				selectedGroup = i
			} else if attacker.getEffectivePower() == mostPower {
				if attacker.initiative > attackingCopy[selectedGroup].initiative {
					selectedGroup = i
				}
			}
		}
		if selectedGroup < 0 {
			log.Panic("Could not determine next group to pick target")
		}
		attacker := attackingCopy[selectedGroup]
		lenBefore := len(defendingCopy)
		target := attacker.selectTarget(&defendingCopy) // May be nil
		if target != nil && len(defendingCopy) != lenBefore-1 {
			log.Panic("Remaining defenders not updated correctly")
		}
		// fmt.Printf("Attacker %v has selected target %v\n", attacker, target)
		targets[attacker] = target
		// Remove selected attacking group from list of potential attackers
		attackingCopy = append(
			attackingCopy[:selectedGroup], attackingCopy[selectedGroup+1:]...)
	}
	return targets
}

func determineAttackOrder(immuneGroups, infectionGroups *[]*Group) []*Group {
	combinedGroups := make([]*Group, len(*immuneGroups)+len(*infectionGroups))
	copy(combinedGroups[:len(*immuneGroups)], *immuneGroups)
	copy(combinedGroups[len(*immuneGroups):], *infectionGroups)
	sort.Slice(combinedGroups,
		func(i, j int) bool {
			return combinedGroups[i].initiative > combinedGroups[j].initiative
		})
	return combinedGroups
}

func removeDeadGroups(groups *[]*Group) *[]*Group {
	aliveGroups := make([]*Group, 0)
	for _, g := range *groups {
		if g.isAlive() {
			aliveGroups = append(aliveGroups, g)
		}
	}
	return &aliveGroups
}

func getAttributes(attributes string, attributeRE *regexp.Regexp) []DamageType {
	attributesSlice := make([]DamageType, 0)
	attributesStr, _ := utils.ParseString(attributes, attributeRE, 1)
	if len(attributesStr) == 0 {
		return attributesSlice
	}
	scanner := bufio.NewScanner(strings.NewReader(attributesStr[0]))
	scanner.Split(bufio.ScanWords)
	for scanner.Scan() {
		attr := scanner.Text()
		if attr[len(attr)-1] == ',' {
			attr = attr[:len(attr)-1]
		}
		attributesSlice = append(attributesSlice, DamageType(attr))
	}
	return attributesSlice
}

func getWeaknesses(attributes string) []DamageType {
	weaknessRE := regexp.MustCompile("weak to ([a-z, ]+)[;)]")
	return getAttributes(attributes, weaknessRE)
}

func getImmunities(attributes string) []DamageType {
	immunityRE := regexp.MustCompile("immune to ([a-z, ]+)[;)]")
	return getAttributes(attributes, immunityRE)
}

func simulateBattle(immuneGroups, infectionGroups *[]*Group) {
	for i := 0; len(*immuneGroups) > 0 && len(*infectionGroups) > 0; i++ {
		targetsOfImmuneSystem := pickTargets(immuneGroups, infectionGroups)
		targetsOfInfection := pickTargets(infectionGroups, immuneGroups)
		attackOrder := determineAttackOrder(immuneGroups, infectionGroups)
		// fmt.Println("Attack order", attackOrder)
		for _, attacker := range attackOrder {
			if !attacker.isAlive() {
				// Group has been killed earlier this round,
				// so it can't attack
				continue
			}
			defender, present := targetsOfImmuneSystem[attacker]
			if !present {
				defender, present = targetsOfInfection[attacker]
				if !present {
					log.Panic("Could not find target of attacker", attacker)
				}
			}
			if defender != nil {
				// fmt.Printf("Attacker: %v\nDefender: %v\n",
				// 	attacker, defender)
				attacker.attackTarget(defender)
			}
		}
		*immuneGroups = *removeDeadGroups(immuneGroups)
		*infectionGroups = *removeDeadGroups(infectionGroups)
		// fmt.Println()
		// fmt.Println("End of round", i)
		// fmt.Println("Immune system")
		// for _, g := range immuneGroups {
		// 	fmt.Println(g)
		// }
		// fmt.Println("Infection")
		// for _, g := range infectionGroups {
		// 	fmt.Println(g)
		// }
	}
}

func countTotalRemainingUnits(immuneGroups, infectionGroups *[]*Group) int {
	totalRemainingUnits := 0
	for _, group := range append(*immuneGroups, *infectionGroups...) {
		totalRemainingUnits += group.units
	}
	return totalRemainingUnits
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	groupRE := regexp.MustCompile(
		"(\\d+) units each with (\\d+) hit points ([a-z,; ()]*)with an attack that does (\\d+) ([a-z]+) damage at initiative (\\d+)")

	immuneGroups := make([]*Group, 0)
	infectionGroups := make([]*Group, 0)
	var activeSide *[]*Group

	// Create immune system and infection groups
	for scanner.Scan() {
		newLine := scanner.Text()
		// fmt.Println("Parsing line:", newLine)
		if len(newLine) == 0 {
			continue
		}
		if newLine == "Immune System:" {
			activeSide = &immuneGroups
		} else if newLine == "Infection:" {
			activeSide = &infectionGroups
		} else {
			parsedLine, err := utils.ParseString(newLine, groupRE, 6)
			// fmt.Println(parsedLine)
			if err != nil {
				panic(err)
			}
			*activeSide = append(*activeSide, newGroup(parsedLine))
		}
	}

	// fmt.Println("Immune system")
	// for _, g := range immuneGroups {
	// 	fmt.Println(g)
	// }
	// fmt.Println("Infection")
	// for _, g := range infectionGroups {
	// 	fmt.Println(g)
	// }

	// Simulate battles and find minimum attack boost that
	// allows immune system to win
	smallestBoostForImmuneToWin := 1000
	largestBoostForInfecionToWin := 0
	attackBoost := largestBoostForInfecionToWin
	totalRemainingUnits := -1
	for attackBoost != smallestBoostForImmuneToWin {
		immuneCopy := copyGroupsDeep(&immuneGroups)
		infectionCopy := copyGroupsDeep(&infectionGroups)
		boostAttack(&immuneCopy, attackBoost)
		// fmt.Println("Simulating attack boost of", attackBoost)
		simulateBattle(&immuneCopy, &infectionCopy)

		immuneWins, infectionWins := false, false
		if len(immuneCopy) > 0 {
			// fmt.Println("Immune system wins")
			immuneWins = true
		}
		if len(infectionCopy) > 0 {
			// fmt.Println("Infection wins")
			infectionWins = true
		}

		if immuneWins == infectionWins {
			fmt.Println(immuneCopy)
			fmt.Println(infectionCopy)
			log.Panic("Undefined battle outcome")
		}
		totalRemainingUnits = countTotalRemainingUnits(
			&immuneCopy, &infectionCopy)

		if attackBoost == 0 {
			fmt.Println("PART 1")
			fmt.Printf("Remaining units for attack boost of %v: %v\n",
				attackBoost, totalRemainingUnits)
		}

		if immuneWins && attackBoost < smallestBoostForImmuneToWin {
			smallestBoostForImmuneToWin = attackBoost
		} else if infectionWins && attackBoost > largestBoostForInfecionToWin {
			largestBoostForInfecionToWin = attackBoost
		}
		attackBoost = (largestBoostForInfecionToWin+smallestBoostForImmuneToWin)/2 + 1
	}
	fmt.Println("PART 2")
	fmt.Printf("Remaining units for attack boost of %v: %v\n",
		attackBoost, totalRemainingUnits)
}
