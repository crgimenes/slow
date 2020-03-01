package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"time"
)

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

		<-time.After(time.Duration(15) * time.Millisecond)
		fmt.Print(string(c))
	}
}
