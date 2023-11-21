// 비동기-논블라킹
package main

import (
	"fmt"
	"time"
)

func printRunning(done chan bool) {
	for {
		// 1초마다 출력
		fmt.Println("Running...")
		time.Sleep(time.Second)

		// select로 1초마다 done 채널 확인하기
		select {
		case <-done:
			return
		default:
		}
	}
}

func timeOut(seconds int, done chan bool) {
	time.Sleep(time.Duration(seconds) * time.Second)
	// n초 후 done을 true로 변경
	done <- true
}

func runAsync() {
	var n int
	fmt.Print("Enter the value of n: ")
	fmt.Scan(&n)

	// 고루틴을 위한 done 채널 만들기
	done := make(chan bool)

	// 고루틴으로 비동기 실행
	go printRunning(done)

	// 고루틴으로 비동기 실행
	go timeOut(n, done)

	// n초 후(done이 true가 되면) 작업 종료
	select {
	case <-done:
		fmt.Println("Task completed.")
	}
}

func main() {
	runAsync()
}
