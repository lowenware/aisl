# AISL

Asynchronous Internet Server Library provides innovative way of web development.
AISL based applications have built-in web server giving full control of client
serving. All you need to know is a request string? - Start prepare the response 
without waiting for headers and body! Don't need some headers? - Don't save them
in memory! Unwanted content-body? - simply ignore it!

## Documentation

[Hello World](https://lowenware.com/aisl/handbook.html#getting-started) example
and full [API reference](https://lowenware.com/aisl/handbook.html#api-reference)
can be found in an oficial [AISL HandBook](https://lowenware.com/aisl/handbook.html).

## Installation

```
$ make PREFIX=/usr/local
$ sudo make PREFIX=/usr/local install
$ sudo cp libaisl.pc.example /usr/lib/pkgconfig/libaisl.pc
```

ArchLinux users can install from [AUR](https://aur.archlinux.org/packages/aisl-git/) :

```
$ yaourt -S aisl-git
```

## License

AISL is free for both commercial and non-commercial use, being distributed under
terms of [CC BY-ND 4.0](https://creativecommons.org/licenses/by-nd/4.0/).
