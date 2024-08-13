; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "Rename Express Installer"

; The file to write
;OutFile "ezthumb-3.2.3-setup.exe"
!include "nsis_version.outfile"

; The default installation directory
InstallDir "$PROGRAMFILES\Rename Express"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rename Express" "Install_Dir"

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
Section "Rename Express (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "renamex.exe"
  File "renamex_win.exe"
  File "COPYING"
  File "ChangeLog.txt"
  File "README.en.txt"
  File "autotest.sh"
  File "rename.ico"
  File "renamex.1"
  File "renamex.lsm"
  File "renamex.pdf"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Rename Express" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Express" "DisplayName" "Rename Express Uninstaller"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Express" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Express" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Express" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Rename Express"
  CreateShortCut "$SMPROGRAMS\Rename Express\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Rename Express\Rename Express.lnk" "$INSTDIR\renamex_win.exe" "" "$INSTDIR\renamex_win.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rename Express"
  DeleteRegKey HKLM "SOFTWARE\Rename Express"
  DeleteRegKey HKCU "SOFTWARE\Rename Express"

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Rename Express\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Rename Express"
  RMDir "$INSTDIR"

SectionEnd
