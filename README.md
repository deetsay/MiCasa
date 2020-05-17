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

After a successful build you can run:

```
sudo ninja install
```
To copy the compiled program into /usr/local/bin.

Or turn it into a package of your choosing:

```
cpack -G DEB
```

```
cpack -G RPM
```

Compilation was successful on OSX Catalina with commands including, but not
necessarily limited to the following:

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

On Linux, pkg-config is used to find LibVLC, but on OSX, the VLC install through Homebrew (as well as
the regular install from DMG, I guess) installs VLC and associated libraries under /Applications/VLC.app/.
Therefore, even if pkg-config would be available, on APPLE this thing won't use it, but a
[custom cmake-module](https://github.com/deetsay/MiCasa/blob/master/cmake/FindLIBVLC.cmake) instead.

I was able to build it successfully on Windows 10 as well. Maybe things would have been easier with
MinGW and pkg-config, but for some reason I used the C++ compiler+CMake+Ninja from Visual Studio's
command line tools. Then I had to:

* Manually copy all the necessary DLL's into the build/executable directory:

```
libjpeg-9.dll
libpng16-16.dll
libtiff-5.dll
libvlc.dll
libvlccore.dll
libwebp-7.dll
SDL2.dll
SDL2_image.dll
zlib1.dll
```

* Manually set the paths for everything in the environment (after downloading all that of course):

```
LIBVLC_INCLUDE_PATH    C:\GL\LibVLC\include
LIBVLC_LIBRARY_PATH    C:\GL\LibVLC\lib\x64
SDL2DIR                C:\GL\SDL2
SDL2IMAGEDIR           C:\GL\SDL2_image
```

* After that it might work the same as everywhere else:

```
git clone https://github.com/deetsay/MiCasa.git
cd MiCasa
cmake
ninja
```

Good luck!

&copy; Tero Mäyränen 2020

License: [GPLv2](https://github.com/deetsay/MiCasa/blob/master/LICENSE)
