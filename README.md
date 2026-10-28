# Big Ugly Pink Mute Plugin

Its Fruity Mute 2 but it works on everything

| Unmuted | Muted |
| --- | --- |
| ![Unmuted at 100%](docs/ui_on.png) | ![Muted at 30%](docs/ui_muted.png) |

Turn the dial down, the channel mutes. Turn it up, it unmutes. 

You can get some cool effects by muting with this plugin and automating the channel mix.

## Building (Windows)

### Download the installer

No build needed: download **[BigUglyPinkMutePlugin-Setup-1.0.0.exe](https://raw.githubusercontent.com/burnerburnerburnerburnerburnerburner/biguglypinkmuteplugin/main/dist/BigUglyPinkMutePlugin-Setup-1.0.0.exe)** (64-bit Windows) and run it.

- It installs the plugin to `C:\Program Files\Common Files\VST3\Big Ugly Pink Mute Plugin.vst3`, then rescan plugins in your DAW.
- It asks for admin rights, because that folder is system-wide.
- The installer isn't code-signed, so Windows SmartScreen may warn you. Choose **More info → Run anyway**.
- Uninstall from **Settings → Apps → Installed apps**.

### Build from source

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

### Package the installer

With [Inno Setup 6](https://jrsoftware.org/isinfo.php) installed, build the plugin first, then:

```bat
ISCC.exe installer\BigUglyPinkMutePlugin.iss
```

The installer is written to `dist\`. Pass `/DVST3Bundle=<path to .vst3>` if the plugin was built somewhere else.

## Tests

Configure with `-DGLASSMUTE_BUILD_TESTS=ON` to build `GlassMuteTests`, which runs audio through the processor (default state, 50% threshold, silence when muted, save/restore) and renders both UI states to PNGs:

```bat
GlassMuteTests.exe <output-folder>
```

## License

Licensed under the [GNU Affero General Public License v3.0](LICENSE). The plugin is built with [JUCE](https://juce.com), which it uses under the same license.
