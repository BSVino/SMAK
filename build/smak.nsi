!define PRODUCT_NAME "SMAK - Super Model Army Knife"
!define PRODUCT_VERSION ""

!define LOCALDIR "install"
;This value should be set depending on the PC this will be compiled under

Var StartMenuFolder
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "SMAK"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SMAK" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_FINISHPAGE_RUN "$INSTDIR/smak.exe"
!define MUI_FINISHPAGE_LINK "Click here to visit the SMAK website"
!define MUI_FINISHPAGE_LINK_LOCATION  "http://www.getsmak.net/"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!include "MUI2.nsh"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "smak-install.exe"
ShowInstDetails show
InstallDir $PROGRAMFILES\SMAK

Section "-Program Files" FILES
	SetOverwrite on
	SetOutPath "$INSTDIR"
	File /r /x *.svn "${LOCALDIR}\*.*"
SectionEnd

Section -StartMenu
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;Create shortcuts
		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortcut "$SMPROGRAMS\$StartMenuFolder\SMAK.lnk" $INSTDIR\smak.exe
		;CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Desktop Shortcut" SHORTCUT
	SetOutPath "$INSTDIR"
	CreateShortcut "$DESKTOP\SMAK.lnk" $INSTDIR\smak.exe
SectionEnd

LangString DESC_SHORTCUT ${LANG_ENGLISH} "Add a shortcut to your desktop"

; Makes the buttons unclickable for some reason!
;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;	!insertmacro MUI_DESCRIPTION_TEXT ${FILES} "Application files - Required"
;	!insertmacro MUI_DESCRIPTION_TEXT ${SHORTCUT} $(DESC_SHORTCUT)
;!insertmacro MUI_FUNCTION_DESCRIPTION_END
