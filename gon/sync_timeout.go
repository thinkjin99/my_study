// 동기-블라킹
package main

import (
	"fmt"
	"time"
)

// 동기-블라킹에서는 timeout과 print가 따로 돌 수 없음
func printRunning(seconds int) {
	for i := 0; i < seconds; i++ {
		fmt.Println("Running...")
		time.Sleep(time.Second)
	}
}

func runSync() {
	var n int
	fmt.Print("Enter the value of n: ")
	fmt.Scan(&n)

	printRunning(n)

	fmt.Println("Task completed.")
}

func main() {
	runSync()
}
