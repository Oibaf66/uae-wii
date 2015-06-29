This is a port of UAE to Nintendo Wii which enables Amiga games to be played on the Nintendo Wii. See the [Wiibrew page](http://wiibrew.org/wiki/UAE_Wii) for usage instructions and download.

![http://uae-wii.googlecode.com/files/stunt-car-racer-1-800.jpg](http://uae-wii.googlecode.com/files/stunt-car-racer-1-800.jpg)

## Build instructions ##
Building UAE for Wii is pretty straight-forward, but needs some care.

### Preparations ###
You'll need to use [devkitpro](http://wiibrew.org/wiki/DevkitPro) (portlibs included) and the svn version of [SDL-wii](http://sdl-wii.googlecode.com), so follow the instructions to build and install SDL first.

You also need a gcc compiler for the host machine.

### Build for the Wii ###
UAE is built with the `Makefile.wii` makefile:

```
make -f Makefile.wii
```

To get a binary release to install on the Wii SD card, do

```
make -f Makefile.wii dist
```

And untar the resulting `uae-bin.tar.gz` file on your wii. Cleaning is done with

```
make -f Makefile.wii clean
```

### Build for the host machine ###
To build for the host, use the `Makefile.host` makefile:

```
make -f Makefile.host
```

Cleaning is done with

```
make -f Makefile.host clean
```