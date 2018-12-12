package main

import (
	"fmt"
	"github.com/rgambee/aoc2018/utils"
	"log"
	"regexp"
	"sort"
	"strings"
	"time"
)

type LogEntry struct {
	Timestamp time.Time
	Message   string
}

func addToSlice(slice []int, value int, startIndex int) []int {
	for i := startIndex; i < len(slice); i++ {
		slice[i] += value
	}
	return slice
}

func extractTimestamp(line string) (time.Time, string) {
	// Example
	// [1518-09-16 23:57] Guard #1889 begins shift
	timestampStartInd := 1
	timestampStopInd := 17
	messageStartInd := 19
	dateTimeFormat := "2006-01-02 15:04"
	timestamp, err := time.Parse(dateTimeFormat,
		line[timestampStartInd:timestampStopInd])
	if err != nil {
		panic(err)
	}
	return timestamp, line[messageStartInd:]
}

func main() {
	file := utils.OpenFile(utils.GetFilename())
	defer utils.CloseFile(file)
	scanner := utils.GetLineScanner(file)

	logEntries := make([]LogEntry, 0)
	guardIDRegex := regexp.MustCompile(" #([0-9]+) ")

	for scanner.Scan() {
		newLine := scanner.Text()
		timestamp, message := extractTimestamp(newLine)
		logEntries = append(logEntries, LogEntry{timestamp, message})
	}
	sort.Slice(logEntries,
		func(i, j int) bool {
			return logEntries[i].Timestamp.Before(logEntries[j].Timestamp)
		})

	sleepSchedules := make(map[int][]int)
	activeGuardID := -1
	for _, entry := range logEntries {
		message := entry.Message
		minute := entry.Timestamp.Minute()
		if strings.Contains(message, "begins shift") {
			activeGuardID = utils.StringToInt(
				utils.ParseString(message, guardIDRegex, 1)[0])
			_, present := sleepSchedules[activeGuardID]
			if !present {
				sleepSchedules[activeGuardID] = make([]int, 60)
			}
		} else if strings.Contains(message, "falls asleep") {
			sleepSchedules[activeGuardID] = addToSlice(sleepSchedules[activeGuardID], 1, minute)
		} else if strings.Contains(message, "wakes up") {
			sleepSchedules[activeGuardID] = addToSlice(sleepSchedules[activeGuardID], -1, minute)
		} else {
			log.Panic("Unknown entry message: ", message)
		}
	}

	mostMinutesAsleep := -1
	sleepiestGuard := -1
	sleepiestMinuteOverall := -1
	guardForSleepistMinuteOverall := -1
	for guardID, schedule := range sleepSchedules {
		minutesAsleep := utils.SumSlice(&schedule)
		if minutesAsleep > mostMinutesAsleep {
			mostMinutesAsleep = minutesAsleep
			sleepiestGuard = guardID
		}
		sleepiestMinute, _ := utils.FindSliceMax(&schedule)
		if sleepiestMinuteOverall < 0 ||
			guardForSleepistMinuteOverall < 0 ||
			schedule[sleepiestMinute] > sleepSchedules[guardForSleepistMinuteOverall][sleepiestMinuteOverall] {
			sleepiestMinuteOverall = sleepiestMinute
			guardForSleepistMinuteOverall = guardID
		}
	}
	sleepiestGuardSchedule := sleepSchedules[sleepiestGuard]
	sleepiestMinuteForGuard, _ := utils.FindSliceMax(&sleepiestGuardSchedule)

	fmt.Println("PART 1")
	fmt.Println("Sleepiest guard:", sleepiestGuard)
	fmt.Println("Sleepiest minute for that guard:", sleepiestMinuteForGuard)
	fmt.Println("Product:", sleepiestGuard*sleepiestMinuteForGuard)
	fmt.Println()
	fmt.Println("PART 2")
	fmt.Println("Sleepist overall minute:", sleepiestMinuteOverall)
	fmt.Println("Guard for sleepiest overall minute:", guardForSleepistMinuteOverall)
	fmt.Println("Product:", sleepiestMinuteOverall*guardForSleepistMinuteOverall)
}
