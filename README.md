# CPU Emulators

![attractdemo](/screens/attractscreen.png)

Very much WIP. But now in playable state. The initial Intel 8080 CPU is done.
Writing a simple test suite helped find a ton of bugs. Hardware emulation is
still in progress. Timing and refresh rate is off.

I'm using SDL3 as a vendored git submodule.


## Building
- cmake
- build-essential (probably)

```bash
./build.sh && build/sdl_8080
```

TODOs:
- [ ] Fix Timing, Refresh Rate.
- [ ] Scale scale OR stretch blitting. (Current window is very small)
- [ ] Sound Emulation
- [ ] Threading? Good chance to dabble with here.

Copyright Dean Martin
