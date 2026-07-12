; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

#define MyAppName "发声键盘-KeyBonk"
#define MyAppVersion "1.4.0.0"
#define MyAppPublisher "小狄同学呀"
#define MyAppURL "https://keyBonk-org.github.io/"
#define MyAppExeName "KeyBonk.exe"
; 架构控制
#ifndef MyAppArch ;64或32，默认64
  #define MyAppArch "64"
#endif
;架构的文字版本
#ifndef MyAppArchStr
  #if MyAppArch == "64"
    #define MyAppArchStr "x64"
  #elif MyAppArch == "32"
    #define MyAppArchStr "x86"
  #endif
#endif
#define DefaultAudioLib "bin\default"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (若要生成新的 GUID，可在菜单中点击 "工具|生成 GUID"。)
AppId={{4A64AE9D-C7CE-4B6D-B022-26408B9412A3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\KeyBonk
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE
; 以下行取消注释，以在非管理安装模式下运行（仅为当前用户安装）。
;PrivilegesRequired=lowest
OutputDir=.\build\installer\
OutputBaseFilename=installer-{#MyAppArchStr}
SetupIconFile=.\resource\ico\64.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: ".\build\{#MyAppArch}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\resource\audios\*"; DestDir: "{app}\{#DefaultAudioLib}\audios"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\resource\background.png"; DestDir: "{app}\{#DefaultAudioLib}"; Flags: ignoreversion
Source: ".\resource\icon-org.png"; DestDir: "{app}\{#DefaultAudioLib}"; Flags: ignoreversion
; 注意: 不要在任何共享系统文件上使用"Flags: ignoreversion"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent