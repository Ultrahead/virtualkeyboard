# VIRTUAL KEYBOARD
## _For the Trimui Smart Pro ..._

Virtual Keyboard is an On-Screen Keyboard (OSK) tailored for the __Trimui Smart Pro__, build on top of ```SDL 2.26.3``` (no Xorg or Wayland support). The keyboard is a refactored version of the one included in  [rs97-commander-sdl2](https://github.com/zfteam/rs97-commander-sdl2). And has been tailored to work in "[Sistema Base de Javier](https://web.telegram.org/a/#-1002392206061_143)", but I guess it may also work in other mods (btw, thanks __Javier__ for such an explendid work adding great value to the stock UI of the console!).

Please note that it will probably work out-of-the-box in the __Trimui Brick__, but in case it doesn't, the code is structured in a way to make it work with little-to-none changes. And even though many of its parts are self-explanatory, I have documented the code following Doxygen's traditional standards for C++, and also included explanations on each key function. It may also work for other linux-based devices for retro-games but I cannot gurantee that it does.

## Features

Virtual Keyboard is a handy OSK that provides the following features:

- It can be executed form any terminal/commandline application.
- It can be called from bash/bat scripts with simple syntax. 
- It comes by default with an English-based QWERTY layout.
- It has two sets of keys to choose from and use, each including special UTF8 characters.
- You can pass the initial text string to display as one of its argument when executing it.
- When the user presses the [OK] button, the keyboard will output the final text string to the console wrapped in [VKStart] and [VKEnd] blocks.

![](https://github.com/Ultrahead/virtualkeyboard/blob/main/screenshots/gif_virtualkeyboard.gif)

(note: the binary has been mainly tested in and ARM64/aarch64 Linux distro, but it also works in Windows 10/11, and it may also work in MacOSX)

In addition to that, you may modify the code to add more rows, columns and charsets, or remove them, by using the ```macros``` and ```constant expressions``` provided.

## Building Steps

First of all, make sure that you have installed in your system all the usual packages and compilers required to compile C++ applications supporting at least the C++11 standard. Also, don't forget to download and install on your system the development packages of SDL2, including the ones supporting, TTF (sdl2_ttf), Image (sdl2_image) and Gfx (sdl2_gfx). It is recommended to browse for v2.26.3 of SDL2, so as to be sure it will run on the TSP. For your convenience I have included a zip file containing such version of SDL2. 

Please notice that the name of the packages and pack managers may vary per distro, but in case of Linux it is important that you compile the code for GNU C Library upto v2.33, which is the one the TSP OS was built for. You can use a Docker Image like Debian:11.11 for Arm64v8 for the building process and it will work.

Once you have your ```Linux``` building environment properly setup, follow these simple steps to build the keyboard:

```sh
git clone https://github.com/Ultrahead/virtualkeyboard.git
cd /virtualkeyboard
make -j $(nproc)
```

In case you need to star over use type ```./make clean``` before you call ```make```. That will remove any previous configuration, 'make' leftovers and VirtualKeyboard app generated in previous builds. 

Finally, for those using Visual Studio, Rider or any other IDE that can open VS solutions, I have included a solution file. If you use it, don't forget to configure your IDE so that both, the compiler and the linker finds the SDL2 SDK to use.

## Installation

There is no need to install the program in the OS that you use to build the virtual keyboard, since the app will be output to the corresponding directory. For example, in Linux, you will find a file named 'VirtualKeyboard' in the same folder where the make files are located.

To install it on the TSP, or the Brick, just copy such file to any APPs folder that you usually use and then execute from the Terminal or a bash script, by following these criteria: 

```sh
sh ./VirtualKeyboard
sh ./VirtualKeyboard "sometexture.png"
sh ./VirtualKeyboard "sometexture.png" "myFont.ttf"
sh ./VirtualKeyboard "sometexture.png" "myFont.ttf" "text to initially show in the keyboard"
```

The first command will open the keyboard with blank text and using a default background texture provided you add one in the "res" folder, naming it "background_default.png". If it fails to load say texture a default grey-like color will be used, instead.

The second command will attempt this time to load the texture "sometexture.png" located in the "res" folder, and again, if it fails, a default grey-like color will be used, instead.

The third command will do the same as before, but in this case, it will also load the font "myFont.ttf" located in the "res" folder, and if it fails, it will stop execution. If nothing is provided (or "" is added so as to input a text parameter), the default font will be loaded, instead.

The fourth command will do the same as before, but in this case, it will show the passed text, that is, "text to initially show in the keyboard" when the keybaord starts. Below, it's an example of say command execution with the text "Hello world!".

![](https://github.com/Ultrahead/virtualkeyboard/blob/main/screenshots/img_helloworld.png)

You can even do things like the following to get the text input by the user ...:

```sh
var=$("$BIN_DIR/VirtualKeyboard" "./background.png" "A:\\$romNameNoExtension.PRG")
```

... and the fetch the text with bash commands like ```sed```:

```sh
if [ $? -eq 0 ]; then
  result="$(echo "$var" | grep -m 1 "VKStar" | sed -e "s/^.*\[VKStart]\([^\"]*\)\[VKEnd].*$/\1/i")"
fi
```

... or commands like ```grep```:

```sh
if [ $? -eq 0 ]; then
  result=$(echo $var | ./grep -oP '(?<=\[VKStart\]).*(?=\[VKEnd\])')
fi
```

In any case, make sure the binaries you use to filter the output and grab the text do support all features. In the last example above, the ```grep``` version that ```BusyBox``` provides does not support the option ```-P``` (note: you can always use another version located in a specific folder and call that one, instead).

In the releases page you will also find a zip file containing and example folder that you can copy paste to your APPs folder in the TSP, provided you use Javier's version, and it will show up on your App section like shown in the image below ...

Just click on that icon so that you can test the keyboard.

![](https://github.com/Ultrahead/virtualkeyboard/blob/main/screenshots/img_icon.png)

## Controls

Controls are mapped as follows on the TSP (and I guess also on the Brick):

|CONTROL|ACTION|
|-----|----------------|
|D-pad| Move the cursor up, down, left and right|
|Button [A]| Presses a key in the keyboard |
|Button [SELECT]| Presses the 'OK' button in the keyboard |
|Button [MENU]| Presses the 'CANCEL' button in the keyboard |
|Button [L]| Moves the caret in the input text to the left |
|Button [R]| Moves the caret in the input text to the right |
|Button [L2]| Moves the cursor to the first key (left) on the same row in the keyboard |
|Button [R2]| Moves the cursor to the last key (right) on the same row in the keyboard |
|Button [Y]| Presses the 'backspace' button on the top-right part of the keyboard |
|Button [X]| Adds a space character (" ") in the text at the current caret's position |

In case you didn't spot it from the above list, the 'START' button is not used, and that is on purpouse. The reason behind it is to let a button free so that other apps can use it for special tasks while the keyboard is running. If you think this is not needed or has no use, talk to Javier ... 

## Final Words ...

This is very important to keep in mind: <ins>the code comes as is with no support</ins>. So, in case you want to modify it to suit your needs, go ahead, grab the code and have fun experimenting with it!

Enjoy .. 🍻
