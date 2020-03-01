package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"time"

	"github.com/crgimenes/goconfig"
)

type config struct {
	BPS int `cfg:"bps" cfgDefault:"300"`
}

func main() {
	var (
		err error
		c   rune
	)

	cfg := config{}
	err = goconfig.Parse(&cfg)

	reader := bufio.NewReader(os.Stdin)
	for {
		c, _, err = reader.ReadRune()
		if err != nil {
			if err == io.EOF {
				break
			}
			println(err)
		}

		<-time.After(time.Second / time.Duration(cfg.BPS/8))
		fmt.Print(string(c))
	}
}
