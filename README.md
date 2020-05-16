Clone and build on Linux:

```
git clone https://github.com/deetsay/MiCasa.git
cd MiCasa
cmake
ninja
```

OR if you want to keep the source directory slightly cleaner:

```
git clone https://github.com/deetsay/MiCasa.git
cd MiCasa
mkdir -p build
cd build
cmake ..
ninja
```

Compilation was successful on OSX Catalina with commands including, but not
necessarily limited to the following (currently experiencing inconveniences
locating libvlc at runtime, negatively affecting ability to manifest moving
pictures):

```
brew install sdl2
brew install sdl2_image
brew cask install vlc
git clone https://github.com/deetsay/MiCasa.git
cd MiCasa
mkdir -p build
cd build
cmake ..
ninja
```

&copy; Tero Mäyränen 2020
License: [GPLv2]{https://github.com/deetsay/MiCasa}
