; Inno Setup script: installs the VST3 bundle into the system VST3 folder.
; Build the plugin first (build.bat), then: ISCC.exe installer\BigUglyPinkMutePlugin.iss

#define AppName "Big Ugly Pink Mute Plugin"
#define AppVersion "1.0.0"

#ifndef VST3Bundle
  #define VST3Bundle GetEnv("LOCALAPPDATA") + "\Temp\gm\build\GlassMute_artefacts\Release\VST3\" + AppName + ".vst3"
#endif

; /DTestInstallDir=<folder> builds a no-admin variant that installs there, for testing.
#ifdef TestInstallDir
  #define VST3Folder TestInstallDir
  #define Privileges "lowest"
#else
  #define VST3Folder "{commoncf64}\VST3"
  #define Privileges "admin"
#endif

[Setup]
AppId={{A2E20C4D-F72B-46F3-81CD-310D8F8C828C}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Glass Audio
; Holds only the uninstaller; the plugin itself goes to Common Files\VST3.
DefaultDirName={autopf}\Glass Audio\{#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired={#Privileges}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=BigUglyPinkMutePlugin-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}
CloseApplications=no

[Files]
Source: "{#VST3Bundle}\*"; DestDir: "{#VST3Folder}\{#AppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
FinishedLabel=Setup has installed [name] into Common Files\VST3. Rescan plugins in your DAW to find it.
