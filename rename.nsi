; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "Rename Extension Installer"

; The file to write
;OutFile "ezthumb-3.2.3-setup.exe"
!include "nsis_version.outfile"

; The default installation directory
InstallDir "$PROGRAMFILES\Rename Extension"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rename Extension" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Rename Extension (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "renamex.exe"
  File "renamex_win.exe"
  File "renamex.1"
  File "renamex.pdf"
  File "rename.ico"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Rename Extension" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Extension" "DisplayName" "Rename Extension Uninstaller"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Extension" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Extension" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Extension" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Rename Extension"
  CreateShortCut "$SMPROGRAMS\Rename Extension\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Rename Extension\Rename Extension.lnk" "$INSTDIR\renamex_win.exe" "" "$INSTDIR\renamex_win.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Extension"
  DeleteRegKey HKLM "SOFTWARE\Rename Extension"
  DeleteRegKey HKCU "SOFTWARE\Rename Extension"

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Rename Extension\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Rename Extension"
  RMDir "$INSTDIR"

SectionEnd
