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
	BPS      int    `cfg:"bps" cfgDefault:"300"`
	FileName string `cfg:"f" cfgDefault:"-"`
}

func closer(c io.Closer) {
	err := c.Close()
	if err != nil {
		fmt.Println("error closing file", err)
	}
}

func main() {
	var (
		err error
		c   rune
		f   *os.File
	)

	cfg := config{}
	goconfig.PrefixEnv = "SLOW"

	err = goconfig.Parse(&cfg)
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	f = os.Stdin

	if cfg.FileName != "-" {
		f, err = os.Open(cfg.FileName)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		defer closer(f)
	}

	reader := bufio.NewReader(f)

	for {
		c, _, err = reader.ReadRune()
		if err != nil {
			if err == io.EOF {
				break
			}

			fmt.Println(err)
		}

		<-time.After(time.Second / time.Duration(cfg.BPS/8))
		fmt.Print(string(c))
	}
}
