package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"time"
)

var bps = 300

func main() {
	var (
		err error
		c   rune
	)
	reader := bufio.NewReader(os.Stdin)
	for {
		c, _, err = reader.ReadRune()
		if err != nil {
			if err == io.EOF {
				break
			}
			println(err)
		}

		<-time.After(time.Second / time.Duration(bps/8))
		fmt.Print(string(c))
	}
}
