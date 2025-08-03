# Decreases the output speed

*Slow* is a simple program that reduces the output speed of programs on the terminal. The goal is to give the feeling of an old modem connection.

The default output speed is *300bps* (bits per second) but you can change it using the *-bps* parameter with either a numeric value or historical modem presets.

## Features

- **Historical Modem Presets**: Use classic modem speeds like `acoustic`, `dialup`, `1200`, `2400`, etc.
- **Accurate Timing**: Improved calculation for precise bit-per-second simulation
- **Better Error Handling**: Graceful shutdown with Ctrl+C and comprehensive error messages
- **Verbose Mode**: Display timing statistics and processing information
- **UTF-8 Compatible**: Handles Unicode characters properly

## Usage Examples

Decreases the output speed of the `ls -al` command to *300bps*.

```console
ls -al | slow
```

Decreases the output speed using a classic 1200 bps modem preset.

```console
cat asciiart.txt | slow -bps 1200
```

Print file at dialup speed (56k) with statistics.

```console
slow -f asciiart.txt -bps dialup -verbose
```

Use acoustic coupler preset (300 bps).

```console
echo "Hello from 1970s!" | slow -bps acoustic
```

Show help with all available presets.

```console
slow -help
```

## Modem Presets

| Preset    | Speed   | Description        |
|-----------|---------|-------------------|
| 110       | 110     | Teletype          |
| 300       | 300     | Acoustic coupler  |
| 1200      | 1200    | Bell 212A         |
| 2400      | 2400    | V.22bis           |
| 4800      | 4800    | V.27              |
| 9600      | 9600    | V.32              |
| 14400     | 14400   | V.32bis           |
| 19200     | 19200   | V.32fast          |
| 28800     | 28800   | V.34              |
| 33600     | 33600   | V.34+             |
| 56000     | 56000   | V.90 (56k)        |
| acoustic  | 300     | Alias for 300     |
| dialup    | 56000   | Alias for 56k     |

## Install

### Install via golang

```console
go install crg.eti.br/go/slow
```

### Build from source

```console
git clone <repository>
cd slow
go build .
```

## Dependencies

This program uses only Go's standard library - no external dependencies required!

## Comments

The *slow* is compatible with UTF-8, and the difference in the size of the symbols is purposely not taken into account. Instead, we simulate what it would be like if the output via code page with each character occupying only 1 byte.
