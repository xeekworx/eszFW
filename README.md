# eszFW

<p>
<a href="https://github.com/mupfelofen-de/eszFW">
  <img src="https://img.shields.io/badge/project-GitHub-blue?style=flat?svg=true" alt="GitHub project" />
</a>
<a href="https://github.com/mupfelofen-de/eszFW/blob/master/LICENCE.md">
  <img src="https://img.shields.io/badge/licence-BEER--WARE-blue?style=flat?svg=true" alt="Licence" />
</a>
<a href="https://ci.appveyor.com/project/mupfelofen-de/eszfw">
  <img src="https://ci.appveyor.com/api/projects/status/0t2yt05ngahfa5jr?svg=true" alt="Build status" />
</a>
<a href="https://lgtm.com/projects/g/mupfelofen-de/eszFW/context:cpp">
<img alt="Language grade: C/C++" src="https://img.shields.io/lgtm/grade/cpp/g/mupfelofen-de/eszFW.svg?logo=lgtm&logoWidth=18?style=flat?svg=true"/>
</a>
</p>

## About

eszFW is a cross-platform game development framework written in C99.
It's mainly aimed at platformer games, but can be used and customized
for any other genre.  Furthermore, this project is the logical
continuation of my older projects [Rainbow
Joe](https://github.com/mupfelofen-de/rainbow-joe) and [Boondock
Sam](https://github.com/mupfelofen-de/boondock-sam).

It is written in C99 using SDL2 and the [TMX Map
Format](http://doc.mapeditor.org/en/stable/reference/tmx-map-format/)
(Tile Map XML).

## Documentation

The documentation can be generated using Doxygen:
```
doxygen
```

A automatically generated version of the documentation can be found
here:  [eszfw.de](https://eszfw.de)

### Status

This project is in development.  I will enhance it gradually, mainly as
a tool to my own use.  To see it in action, take a look at [Tau
Ceti](https://github.com/mupfelofen-de/TauCeti): a cross-platform 2D
sci-fi themed tech demo which I develop to test this framework.

[![Tau Ceti](https://media.eszfw.de/tc-01-tn.png)](https://media.eszfw.de/tc-01.png?raw=true "Tau Ceti 1")
[![Tau Ceti](https://media.eszfw.de/tc-02-tn.png)](https://media.eszfw.de/tc-02.png?raw=true "Tau Ceti 2")

An Android version of Tau Ceti is available on Google Play:

[![Get it on Google Play](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=de.mupfelofen.TauCeti)

### Trivia

The abbreviation esz is a tribute to my best friend [Ertugrul
Söylemez](https://github.com/esoeylemez), who suddenly passed away on
May 12th, 2018.  We all miss you deeply.

## Why C?

A question I hear alot is why I'm using C99 for this project instead of
a modern language like Rust, Haskell or `[insert your preferred language
here]`, because writing games in C99 is neither a easy thing to do nor
very common these days (even though many popular and critical acclaimed
games are written in C e.g. Doom, Quake and Neverwinter Nights).

The main reason is simple: because I share the view that object-oriented
programming creates more problems than it solves. I think that in many
cases, object-oriented programming creates confusing coherences and
delivers the illusion of higher flexibility, structure and
maintainability.  If you are more interested in the topic, take a look at
this video by Brian Will: [Object-Oriented Programming is
Bad](https://www.youtube.com/watch?v=QM1iUe6IofM).

But the more important reason why I chose C99: because I love it.  Even
though I am currently trying to get on with Haskell (and functional
programming in general), I still consider C a fun and challenging
language which I also use professionally.

Anyway: I hope you find the code of this project useful.  If so, feel
free to use it in any way you want. Just consider buying me a beer in
case we meet someday.

## Dependencies

The program has been successfully compiled with the following libraries:
```
libxml2    2.9.8
sdl2       2.0.8
sdl2_image 2.0.3
sdl2_mixer 2.0.2
zlib       1.2.11
```

## Compiling

First clone the repository including the submodules:
```
git clone --recurse-submodules -j2 https://github.com/mupfelofen-de/eszFW.git
cd eszFW
```

To compile _eszFW_ under Linux use CMake as follows:
```
mkdir build
cd build
cmake ..
make
```

## Licence and Credits

This project is licenced under the "THE BEER-WARE LICENCE".  See the
file [LICENCE.md](LICENCE.md) for details.

[TMX C Loader](https://github.com/baylej/tmx/) by Bayle Jonathan is
licenced under a BSD 2-Clause "Simplified" Licence.  See the file
[COPYING](https://raw.githubusercontent.com/baylej/tmx/master/COPYING)
for details.
