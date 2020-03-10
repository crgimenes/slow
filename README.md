# Decreases the output speed

*Slow* is a simple program that reduces the output speed of programs on the terminal. The goal is to give the feeling of an old modem connection.

The default output speed is *300bps* (bits per second) but you can change it using the *-bps* parameter and the speed.

## Esage Examples

Decreases the output speed of the `ls -al` command to *300bps*.

```console
ls -al|slow
```

Decreases the output speed of the `cat` command to *14,400bps*.

```console
cat asciiart.txt|slow -bps 14400
```

Print file at 1,200bps 

```console
slow -f asciiart.txt -bps 1200
```

## Install

### Install via golang

```console
go install github.com/crgimenes/slow
```

## Comments

The *slow* is compatible with UTF-8 and the difference in the size of the symbols is purposely not taken into account. This way we simulate what it would be like if it were the output via code page with each symbol occupying only 1 byte.

