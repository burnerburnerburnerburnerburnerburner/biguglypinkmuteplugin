# Big Ugly Pink Mute Plugin

A VST3 effect with exactly one job: an automatable on/off dial, in the spirit of Fruity Mute 2, dressed in pink liquid glass.

| Unmuted | Muted |
| --- | --- |
| ![Unmuted at 100%](docs/ui_on.png) | ![Muted at 30%](docs/ui_muted.png) |

## How it works

- One parameter, **On/Off**, from 0 to 100%.
- **Below 50%** the signal is muted. **50% and above** it passes through untouched.
- Opens at **100% (unmuted)**.
- The host shows the value as `On` or `Muted`, so automation lanes read clearly.
- Each switch fades over 4 ms, so gating stays tight without clicks.
- Mono and stereo.

## Controls

- **Dial**: drag up/down to turn. Double-click to reset to 100%.
- **Status pill**: click to toggle between muted and unmuted.
- The white bead at twelve o'clock marks the 50% threshold.

## Building (Windows)

Requires CMake 3.22+, Ninja, Git and Visual Studio 2022 (or the Build Tools) with the C++ workload.

```bat
build.bat
```

The script clones JUCE 8.0.4 and builds in `%LOCALAPPDATA%\Temp\gm`, because a deeply nested folder can exceed Windows' 260-character path limit during the build. Set `JUCE_DIR` to use an existing JUCE checkout instead.

The plugin is written to:

```
%LOCALAPPDATA%\Temp\gm\build\GlassMute_artefacts\Release\VST3\Big Ugly Pink Mute Plugin.vst3
```

Copy that folder into `C:\Program Files\Common Files\VST3` (needs admin), then rescan plugins in your DAW.

## Tests

Configure with `-DGLASSMUTE_BUILD_TESTS=ON` to build `GlassMuteTests`, which runs audio through the processor (default state, 50% threshold, silence when muted, save/restore) and renders both UI states to PNGs:

```bat
GlassMuteTests.exe <output-folder>
```
