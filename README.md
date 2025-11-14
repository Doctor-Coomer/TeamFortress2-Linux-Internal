# What is this?
This is a program that is compiled into a [Shared Object](https://en.wikipedia.org/wiki/Shared_library), and is used to give Extrasensory Perception, automate gameplay, and overall enhance the game through removing restrictions and adding extra features, for "Team Fortress 2". In a nutshell, this is a hack/cheat.  
  
Development is intermittent, as there is currently only one person (me) with push and merge rights (a Project Maintainer) at this time. Please read the [I want to contribute!](#-i-want-to-contribute!) section for more detail.  
<b>Maintainer note: I've been especially inactive since August, due to things outside of this project taking priority. Open Issues and Pull requests won't be properly addressed by me for a long time. I am also reachable if you wish to be a dedicated maintainer of the project.</b>
  
Also, this project started as an unfaithful port of [this internal hack](https://github.com/faluthe/tf_c) to C++, but has far surpassed the goal of "just a port".  

# How does this work?
It internally (as a program that is injected inside TF2's process) reads and writes memory to the game. A notable feature of it's functionality is using exposed symbols of other Shared Objects used by the game to run the game's own code against itself. The hack accomplishes this via interfaces of the game's own classes.  
  
For more information about Source Engine interfaces and structures, there is the [Source SDK 2013 Multiplayer](https://github.com/ValveSoftware/source-sdk-2013) from Valve Software.  
  
There are also many more hacks this program does to achieve functionality outside of what the Source SDK intended.  

# How to compile
### Cloning Repository
Clone the repo using `git`.  

```bash
$ git clone https://github.com/Doctor-Coomer/TeamFortress2-Linux-Internal.git
$ cd TeamFortress2-Linux-Internal/
```

### Install Dependencies
Install the necessary packages.  
```bash
$ ./packages/packages.sh
[sudo] password for user:
...
```

### Compiling
Build the program from source.  
```bash
$ ./build.sh
...
```

### Running
Injecting the program into Team Fortress 2.  
```bash
$ sudo ./inject.sh
[sudo] password for user:
...
Library loaded successfully at 0x12345678900. Use Ctrl+C to unload.
```

# Features
(Non-exhaustive list)
* Aimbot
* Player ESP
  - Box
  - Name
  - Health
  - Flags
* Visuals
  - Remove scope overlay
  - Override FOV
* Bhop
* sv_pure bypass

![ESP](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/v2a6lpb3ptrdkftd17.png)
![MENU1](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/ugeypgx5pkunur84ij.png)
![MENU2](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/nrsxdr0lzebtpvfptd.png)
![MENU3](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/xt7xrw2e5opbvb0p7n.png)
![MENU4](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/rofyn5qzedydfh69bh.png)
![MENU5](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/nvj7733dasulmqankh.png)
![MENU6](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/jaax53h7g1ayk13das.png)
![pickups](https://r2.e-z.host/bb3dfc85-7f7f-4dcb-8b0b-3a4af0aa57e4/6jhjaiseb021kvef5b.png)

# I want to contribute!
Hello, I ([Dr_Coomer](https://github.com/Doctor-Coomer)) can be busy outside this project, and so development may not be at a daily or even weekly pace.  
If you want to contribute, please keep these in mind:  
- This project uses snake_case
- This project uses a [Unity Build](https://en.wikipedia.org/wiki/Unity_build) paradigm
- I attempt to prioritize straight forward and simple, yet robust, C++ code <!-- a link to a blog explaining my preferred style of "modern C++" should be added here -->

If you contribute entirely new features and files, I will most likely sit down and rewrite your entire pull request to fit with those points. That isn't a reason to not contribute, but that is a reason why your changes may take a while to add. All contributions are valuable.  

# TODO
* More robust build system
* Name project (?)
* ESP using IMGUI (Eventually?)
* Add Engine Prediction
* Add Dormant ESP fade
* Add more game modes to navbot
* Add roaming to navbot
* Add building aimbot
