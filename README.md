BunnymodXT-Injector
===================

A small simple injector for Bunnymod XT. Injects **BunnymodXT.dll** from the directory where the injector is located.

#How to use

1. Get a Bunnymod XT DLL from [here](https://github.com/YaLTeR/BunnymodXT/releases).
2. Place it into the same folder as the injector.

###Standard way
1. Run the game.
2. Run the injector.

###Game start way (you will have to do steps 1-7 only once)
1. If you don't have a shortcut to your `hl.exe`, create one.
2. Right click on the shortcut and click *Properties*.
3. Copy everything from the *Target* field.
4. You can now delete this shortcut.
5. Create a shortcut to the injector.
6. Go into its *Properties*.
7. Place the cursor in the end of the *Target* field, press space and Ctrl-V (paste), click OK.
8. You can now use this shortcut to start the game with BunnymodXT!

##If you get an error, try launching the injector as admin.

#Additional flags
- **-processname <name>** makes the injector inject into a process with the given name. Example: `-processname hlds.exe`

#Building
You will need at least Visual Studio 2015 CTP.
