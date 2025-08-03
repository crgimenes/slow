package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"
	"time"
)

var (
	bpsFlag     = flag.String("bps", "300", "Set speed in bits per second or use preset")
	fileFlag    = flag.String("f", "-", "Read from file instead of stdin")
	helpFlag    = flag.Bool("help", false, "Show help message")
	verboseFlag = flag.Bool("verbose", false, "Show timing and statistics")
)

// Presets de velocidades históricas de modems
var modemPresets = map[string]int{
	"110":      110,   // Teletype
	"300":      300,   // Acoustic coupler
	"1200":     1200,  // Bell 212A
	"2400":     2400,  // V.22bis
	"4800":     4800,  // V.27
	"9600":     9600,  // V.32
	"14400":    14400, // V.32bis
	"19200":    19200, // V.32fast
	"28800":    28800, // V.34
	"33600":    33600, // V.34+
	"56000":    56000, // V.90
	"acoustic": 300,   // Alias para 300
	"dialup":   56000, // Alias para 56k
}

func closer(c io.Closer) {
	err := c.Close()
	if err != nil {
		fmt.Fprintf(os.Stderr, "error closing file: %v\n", err)
	}
}

func parseBPS(bpsStr string) (int, error) {
	// Primeiro, verifica se é um preset
	if preset, exists := modemPresets[strings.ToLower(bpsStr)]; exists {
		return preset, nil
	}

	// Senão, tenta converter para número
	bps, err := strconv.Atoi(bpsStr)
	if err != nil {
		return 0, fmt.Errorf("invalid BPS value '%s'. Use a number or one of the presets: %s",
			bpsStr, getAvailablePresets())
	}

	if bps <= 0 {
		return 0, fmt.Errorf("BPS must be positive, got %d", bps)
	}

	if bps > 1000000 {
		return 0, fmt.Errorf("BPS too high (max 1,000,000), got %d", bps)
	}

	return bps, nil
}

func getAvailablePresets() string {
	var presets []string
	for key := range modemPresets {
		presets = append(presets, key)
	}
	return strings.Join(presets, ", ")
}

func showHelp() {
	fmt.Printf(`slow - simulates old terminal/modem speeds

Usage:
  slow [options]
  command | slow [options]

Options:
  -bps <speed>     Set speed in bits per second (default: 300)
  -f <file>        Read from file instead of stdin (default: -)
  -verbose         Show timing and statistics
  -help            Show this help

BPS Presets (historical modems):
  110      Teletype
  300      Acoustic coupler (default)
  1200     Bell 212A
  2400     V.22bis
  4800     V.27
  9600     V.32
  14400    V.32bis
  19200    V.32fast
  28800    V.34
  33600    V.34+
  56000    V.90 (56k dialup)
  acoustic Alias for 300
  dialup   Alias for 56000

Examples:
  ls -al | slow
  slow -f text.txt -bps 1200
  cat file.txt | slow -bps dialup
  slow -f story.txt -bps acoustic -verbose
`)
}

func setupSignalHandling() chan os.Signal {
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	return sigChan
}

func main() {
	var (
		err error
		c   rune
		f   *os.File
	)

	// Parse command line flags
	flag.Parse()

	if *helpFlag {
		showHelp()
		return
	}

	// Parse BPS with preset support
	bps, err := parseBPS(*bpsFlag)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	if *verboseFlag {
		fmt.Fprintf(os.Stderr, "Simulating %d bps (%d bytes/sec)\n", bps, bps/8)
	}

	// Setup signal handling for graceful shutdown
	sigChan := setupSignalHandling()

	f = os.Stdin

	if *fileFlag != "-" {
		f, err = os.Open(*fileFlag)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error opening file '%s': %v\n", *fileFlag, err)
			os.Exit(1)
		}
		defer closer(f)
	}

	reader := bufio.NewReader(f)

	// Calcular delay correto: cada byte (8 bits) a X bps
	// delay = (8 bits / bps) * 1 segundo = (8 / bps) segundos
	// Em nanosegundos: (8 * 1e9) / bps
	delayNs := time.Duration((8*1e9)/bps) * time.Nanosecond

	var charCount int64
	startTime := time.Now()

	for {
		c, _, err = reader.ReadRune()
		if err != nil {
			if err == io.EOF {
				if *verboseFlag {
					elapsed := time.Since(startTime)
					fmt.Fprintf(os.Stderr, "\nCompleted: %d characters in %v\n", charCount, elapsed)
				}
				return
			}
			fmt.Fprintf(os.Stderr, "Error reading: %v\n", err)
			os.Exit(1)
		}

		// Usar select para gerenciar tanto o delay quanto os sinais
		select {
		case <-sigChan:
			if *verboseFlag {
				elapsed := time.Since(startTime)
				fmt.Fprintf(os.Stderr, "\nProcessed %d characters in %v\n", charCount, elapsed)
			}
			return
		case <-time.After(delayNs):
			// Delay completado, imprimir o caractere
			fmt.Print(string(c))
			charCount++
		}
	}
}
