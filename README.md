# CPU Emulators

![attractdemo](/screens/attractscreen.png)
![flipped_y](/screens/flipped_y.png)

A work in progress, but in a playable state. The initial Intel 8080 CPU is done.
Writing a simple test suite helped find a ton of bugs. Hardware emulation is
still in progress. Timing and refresh rate is off.

I'm using SDL3 as a vendored git submodule.

## Building
- cmake
- build-essential

```bash
git clone --recurse-submodules https://github.com/dean-martin/cpu-emulators
cd cpu-emulators
./build.sh && build/sdl_8080
```

## Controls
Left, Right, UP/Spacebar for both players  
C to insert credit  
P to Pause  
T for TILT (historical anticheat for pinball, causes GAME OVER)  

TODOs:
- [X] Colors.
- [ ] Fix Timing, Refresh Rate.
- [ ] Scale scale OR stretch blitting. (Current window is very small)
- [ ] Sound Emulation
- [ ] Threading? Good chance to dabble with here.

Copyright Dean Martin
