# gpsim.nsi - NSIS installer script for gpsim
#
# Copyright (c) 2004 Borut Razem
#
# This file is part of gpsim.
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#
#  Borut Razem
#  borut.razem@siol.net

; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "gpsim"
!define PRODUCT_VERSION "0.21.10"
!define PRODUCT_PUBLISHER "www.dattalo.com"
!define PRODUCT_WEB_SITE "http://www.dattalo.com/gnupic/gpsim.html"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\gpsim.bat"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define GPSIM_ROOT "..\.."
!define SETUP_DIR "..\..\..\..\gpsim_snapshots"
!define PKG_ROOT "${SETUP_DIR}\gpsim_pkg"

SetCompressor /SOLID lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_ICON "${PKG_ROOT}\gpsim.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "${GPSIM_ROOT}\COPYING"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\gpsim.bat"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.TXT"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

#!system "lyx -e latex ../../doc/gpsim.lyx" = 0
#!system "pdflatex ../../doc/gpsim.tex" = 0

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${SETUP_DIR}\gpsim-${PRODUCT_VERSION}-YYYYMMDD-setup.exe"
InstallDir "$PROGRAMFILES\gpsim"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  ;Uninstall the old version, if present
  ReadRegStr $R0 HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $R0 "" inst

  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "$(^Name) is already installed. $\n$\nClick 'OK' to remove the \
  previous version or 'Cancel' to cancel this upgrade." \
  IDOK uninst
  Abort

uninst:
  ; Run the uninstaller
  ClearErrors
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file

  ;IfErrors no_remove_uninstaller
  ;  ;You can either use Delete /REBOOTOK in the uninstaller or add some code
  ;  ;here to remove to remove the uninstaller. Use a registry key to check
  ;  ;whether the user has chosen to uninstall. If you are using an uninstaller
  ;  ;components page, make sure all sections are uninstalled.
  ;no_remove_uninstaller:

  Goto done
inst:

  ; Install the new version
  MessageBox MB_YESNO|MB_ICONQUESTION "This will install $(^Name). Do you wish to continue?" IDYES +2
  Abort

done:
FunctionEnd

Section "MainSection" SEC01

  SetOutPath "$INSTDIR\bin"
  SetOverwrite ifnewer
  File "${GPSIM_ROOT}\gpsim\gpsim.exe"
  File "${GPSIM_ROOT}\modules\gpsim_modules.dll"
  File "${GPSIM_ROOT}\plat\win32\modules.def"
  File "${PKG_ROOT}\bin\asprintf.dll"
  File "${PKG_ROOT}\bin\charset.dll"
  File "${PKG_ROOT}\bin\iconv.dll"
  File "${PKG_ROOT}\bin\intl.dll"
  File "${PKG_ROOT}\bin\libatk-1.0-0.dll"
  File "${PKG_ROOT}\bin\libgdk-win32-2.0-0.dll"
  File "${PKG_ROOT}\bin\libgdk_pixbuf-2.0-0.dll"
  File "${PKG_ROOT}\bin\libglib-2.0-0.dll"
  File "${PKG_ROOT}\bin\libgmodule-2.0-0.dll"
  File "${PKG_ROOT}\bin\libgobject-2.0-0.dll"
  File "${PKG_ROOT}\bin\libgthread-2.0-0.dll"
  File "${PKG_ROOT}\bin\libgtk-win32-2.0-0.dll"
  File "${PKG_ROOT}\bin\libiconv-2.dll"
  File "${PKG_ROOT}\bin\libintl-2.dll"
  File "${PKG_ROOT}\bin\libpango-1.0-0.dll"
  File "${PKG_ROOT}\bin\libpangoft2-1.0-0.dll"
  File "${PKG_ROOT}\bin\libpangowin32-1.0-0.dll"
  File "${PKG_ROOT}\bin\popt1.dll"
  File "${PKG_ROOT}\bin\pthreadVC2.dll"
  File "${PKG_ROOT}\bin\readline.dll"

  SetOutPath "$INSTDIR\doc"
  File "${GPSIM_ROOT}\doc\gpsim_cvs.html"
  File "${GPSIM_ROOT}\doc\gpsimWin32.html"
  File "${GPSIM_ROOT}\doc\gpsim.pdf"

  SetOutPath "$INSTDIR\etc\gtk-2.0"
  File "${PKG_ROOT}\etc\gtk-2.0\gdk-pixbuf.loaders"

  SetOutPath "$INSTDIR\etc\pango"
  File "${PKG_ROOT}\etc\pango\pango.aliases"
  File "${PKG_ROOT}\etc\pango\pango.modules"

  SetOutPath "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-am-et.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-ti-et.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-ti-er.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-thai-broken.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-ipa.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-inuktitut.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-cyrillic-translit.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-cedilla.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\immodules\im-viqr.dll"

  SetOutPath "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-xpm.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-xbm.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-wbmp.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-tiff.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-tga.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ras.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-pnm.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-png.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-pcx.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-jpeg.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ico.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-gif.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-bmp.dll"
  File "${PKG_ROOT}\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ani.dll"

  SetOutPath "$INSTDIR\lib\pango\1.4.0\modules"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-thai-fc.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-indic-fc.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-hebrew-fc.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-hangul-fc.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-basic-win32.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-basic-fc.dll"
  File "${PKG_ROOT}\lib\pango\1.4.0\modules\pango-arabic-fc.dll"

  SetOutPath "$INSTDIR\lib\gpsim"
  File "${GPSIM_ROOT}\gpsim\gpsim.lib"
  File "${GPSIM_ROOT}\plat\win32\modules.def"

  SetOutPath "$INSTDIR\"
  File /oname=ChangeLog.txt "${GPSIM_ROOT}\ChangeLog"
  File "${GPSIM_ROOT}\plat\win32\gpsim.ico"
  File /oname=COPYING.TXT "${GPSIM_ROOT}\COPYING"
  File /oname=README.TXT "${GPSIM_ROOT}\README"

  SetOutPath "$INSTDIR\include\gpsim"
  File "${GPSIM_ROOT}\src\*.h"

  SetOutPath "$INSTDIR\examples\12bit"
  File "${GPSIM_ROOT}\examples\12bit\*.asm"
  File "${GPSIM_ROOT}\examples\12bit\*.inc"
  File "${GPSIM_ROOT}\examples\12bit\*.stc"

  SetOutPath "$INSTDIR\examples\14bit"
  File "${GPSIM_ROOT}\examples\14bit\*.asm"
  File "${GPSIM_ROOT}\examples\14bit\*.stc"

  SetOutPath "$INSTDIR\examples\16bit"
  File "${GPSIM_ROOT}\examples\16bit\*.asm"
  File "${GPSIM_ROOT}\examples\16bit\*.inc"

  SetOutPath "$INSTDIR\examples\modules\led_test"
  File "${GPSIM_ROOT}\examples\modules\led_test\*.asm"
  File "${GPSIM_ROOT}\examples\modules\led_test\*.stc"

  SetOutPath "$INSTDIR\examples\modules\logic_test"
  File "${GPSIM_ROOT}\examples\modules\logic_test\Makefile"
  File "${GPSIM_ROOT}\examples\modules\logic_test\*.asm"
  File "${GPSIM_ROOT}\examples\modules\logic_test\*.stc"

  SetOutPath "$INSTDIR\examples\modules\mod_test"
  File "${GPSIM_ROOT}\examples\modules\mod_test\Makefile"
  File "${GPSIM_ROOT}\examples\modules\mod_test\*.asm"
  File "${GPSIM_ROOT}\examples\modules\mod_test\*.stc"

  SetOutPath "$INSTDIR\examples\modules\paraface_test"
  File "${GPSIM_ROOT}\examples\modules\paraface_test\Makefile"
  File "${GPSIM_ROOT}\examples\modules\paraface_test\*.asm"
  File "${GPSIM_ROOT}\examples\modules\paraface_test\*.stc"

  SetOutPath "$INSTDIR\examples\modules\usart_test"
  File "${GPSIM_ROOT}\examples\modules\usart_test\Makefile"
  File "${GPSIM_ROOT}\examples\modules\usart_test\*.asm"
  File "${GPSIM_ROOT}\examples\modules\usart_test\*.stc"

  SetOutPath "$INSTDIR\examples\projects"
  File "${GPSIM_ROOT}\examples\projects\README"

  SetOutPath "$INSTDIR\examples\projects\digital_stim"
  File "${GPSIM_ROOT}\examples\projects\digital_stim\Makefile"
  File "${GPSIM_ROOT}\examples\projects\digital_stim\ChangeLog"
  File "${GPSIM_ROOT}\examples\projects\digital_stim\README"
  File "${GPSIM_ROOT}\examples\projects\digital_stim\digital_stim.asm"
  File "${GPSIM_ROOT}\examples\projects\digital_stim\digital_stim.stc"

  SetOutPath "$INSTDIR\examples\projects\p16f628_test"
  File "${GPSIM_ROOT}\examples\projects\p16f628_test\ChangeLog"
  File "${GPSIM_ROOT}\examples\projects\p16f628_test\Makefile"
  File "${GPSIM_ROOT}\examples\projects\p16f628_test\README"
  File "${GPSIM_ROOT}\examples\projects\p16f628_test\f628.asm"
  File "${GPSIM_ROOT}\examples\projects\p16f628_test\f628.stc"

  SetOutPath "$INSTDIR\examples\projects\stack_test"
  File "${GPSIM_ROOT}\examples\projects\stack_test\ChangeLog"
  File "${GPSIM_ROOT}\examples\projects\stack_test\Makefile"
  File "${GPSIM_ROOT}\examples\projects\stack_test\README"
  File "${GPSIM_ROOT}\examples\projects\stack_test\stack_test.asm"
  File "${GPSIM_ROOT}\examples\projects\stack_test\stack_test.stc"

  SetOutPath "$INSTDIR\examples\scripts"
  File "${GPSIM_ROOT}\examples\scripts\makefile"
  File "${GPSIM_ROOT}\examples\scripts\README"
  File "${GPSIM_ROOT}\examples\scripts\*.cc"
  File "${GPSIM_ROOT}\examples\scripts\*.h"
  File "${GPSIM_ROOT}\examples\scripts\*.asm"
  File "${GPSIM_ROOT}\examples\scripts\*.py"

  SetOutPath "$INSTDIR\extras\lcd\examples"
  File "${GPSIM_ROOT}\extras\lcd\examples\Makefile.am"
  File "${GPSIM_ROOT}\extras\lcd\examples\README"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcd.asm"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcd.inc"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcd_mod.asm"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcd_mod.stc"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcdmemtest.c"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcdmemtest.hex"
  File "${GPSIM_ROOT}\extras\lcd\examples\lcdmemtest.stc"
  File "${GPSIM_ROOT}\extras\lcd\examples\p16c64.inc"
  File "${GPSIM_ROOT}\extras\lcd\examples\p16c84.inc"
  File "${GPSIM_ROOT}\extras\lcd\examples\screen.asm"
  File "${GPSIM_ROOT}\extras\lcd\examples\screen.inc"

  SetOutPath "$INSTDIR\extras\lcd"
  File "${GPSIM_ROOT}\extras\lcd\AUTHORS"
  File "${GPSIM_ROOT}\extras\lcd\COPYING"
  File "${GPSIM_ROOT}\extras\lcd\ChangeLog"
  File "${GPSIM_ROOT}\extras\lcd\INSTALL"
  File "${GPSIM_ROOT}\extras\lcd\Makefile.am"
  File "${GPSIM_ROOT}\extras\lcd\makefile.mingw"
  File "${GPSIM_ROOT}\extras\lcd\NEWS"
  File "${GPSIM_ROOT}\extras\lcd\README"
  File "${GPSIM_ROOT}\extras\lcd\autogen.sh"
  File "${GPSIM_ROOT}\extras\lcd\caps.pl"
  File "${GPSIM_ROOT}\extras\lcd\configure.in"
  File "${GPSIM_ROOT}\extras\lcd\lcd.cc"
  File "${GPSIM_ROOT}\extras\lcd\lcd.gif"
  File "${GPSIM_ROOT}\extras\lcd\lcd.h"
  File "${GPSIM_ROOT}\extras\lcd\lcd.xpm"
  File "${GPSIM_ROOT}\extras\lcd\lcdengine.cc"
  File "${GPSIM_ROOT}\extras\lcd\lcdfont.h"
  File "${GPSIM_ROOT}\extras\lcd\lcdfont.inc"
  File "${GPSIM_ROOT}\extras\lcd\lcdgui.cc"
  File "${GPSIM_ROOT}\extras\lcd\module_manager.cc"
  File "${GPSIM_ROOT}\extras\lcd\t.gif"

  SetOutPath "$INSTDIR\extras\rs232-gen\example"
  File "${GPSIM_ROOT}\extras\rs232-gen\example\Makefile"
  File "${GPSIM_ROOT}\extras\rs232-gen\example\README"
  File "${GPSIM_ROOT}\extras\rs232-gen\example\example.asm"
  File "${GPSIM_ROOT}\extras\rs232-gen\example\example.stc"

  SetOutPath "$INSTDIR\extras\rs232-gen"
  File "${GPSIM_ROOT}\extras\rs232-gen\Makefile"
  File "${GPSIM_ROOT}\extras\rs232-gen\README"
  File "${GPSIM_ROOT}\extras\rs232-gen\rs232-gen.c"

SectionEnd

Section -Icons
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"

  Call CreateBatFile
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"

  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.bat" "" "$INSTDIR\gpsim.ico" "" "" "" ""
;  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.bat" "" "$INSTDIR\gpsim.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME} on the Web.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Documentation.lnk" "$INSTDIR\doc\gpsim.pdf" "" "$INSTDIR\gpsim.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Change Log.lnk" "$INSTDIR\ChangeLog.txt" "" "$INSTDIR\gpsim.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\README.lnk" "$INSTDIR\README.TXT" "" "$INSTDIR\gpsim.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\GPL 2 License.lnk" "$INSTDIR\COPYING.TXT"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\gpsim.bat"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Path" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\gpsim.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\.gpsim"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\README.TXT"
  Delete "$INSTDIR\COPYING.TXT"
  Delete "$INSTDIR\gpsim.bat"
  Delete "$INSTDIR\gpsim.ico"
  Delete "$INSTDIR\ChangeLog.txt"

  Delete "$INSTDIR\bin\gpsim_modules.dll"
  Delete "$INSTDIR\bin\modules.def"

  Delete "$INSTDIR\examples\12bit\*.*"
  Delete "$INSTDIR\examples\14bit\*.*"
  Delete "$INSTDIR\examples\16bit\*.*"

  Delete "$INSTDIR\examples\modules\led_test\*.*"
  Delete "$INSTDIR\examples\modules\logic_test\*.*"
  Delete "$INSTDIR\examples\modules\mod_test\*.*"
  Delete "$INSTDIR\examples\modules\paraface_test\*.*"
  Delete "$INSTDIR\examples\modules\usart_test\*.*"

  Delete "$INSTDIR\examples\projects\digital_stim\*.*"
  Delete "$INSTDIR\examples\projects\p16f628_test\*.*"
  Delete "$INSTDIR\examples\projects\stack_test\*.*"
  Delete "$INSTDIR\examples\projects\*.*"
  Delete "$INSTDIR\examples\scripts\*.*"

  Delete "$INSTDIR\extras\lcd\examples\*.*"
  Delete "$INSTDIR\extras\lcd\*.*"
  Delete "$INSTDIR\extras\rs232-gen\example\*.*"
  Delete "$INSTDIR\extras\rs232-gen\*.*"

  Delete "$INSTDIR\lib\gpsim\gpsim.lib"
  Delete "$INSTDIR\lib\gpsim\modules.def"

  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-arabic-fc.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-basic-fc.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-basic-win32.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-hangul-fc.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-hebrew-fc.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-indic-fc.dll"
  Delete "$INSTDIR\lib\pango\1.4.0\modules\pango-thai-fc.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-ani.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-bmp.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-gif.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-ico.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-jpeg.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-pcx.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-png.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-pnm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-ras.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-tga.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-tiff.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-wbmp.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-xbm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\cygpixbufloader-xpm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ani.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-bmp.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-gif.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ico.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-jpeg.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-pcx.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-png.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-pnm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-ras.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-tga.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-tiff.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-wbmp.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-xbm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\loaders\libpixbufloader-xpm.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-viqr.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-cedilla.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-cyrillic-translit.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-inuktitut.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-ipa.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-thai-broken.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-ti-er.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-ti-et.dll"
  Delete "$INSTDIR\lib\gtk-2.0\2.4.0\immodules\im-am-et.dll"

  Delete "$INSTDIR\etc\pango\pango.modules"
  Delete "$INSTDIR\etc\pango\pango.aliases"
  Delete "$INSTDIR\etc\gtk-2.0\gtkrc"
  Delete "$INSTDIR\etc\gtk-2.0\gdk-pixbuf.loaders"

  Delete "$INSTDIR\doc\gpsim.pdf"
  Delete "$INSTDIR\doc\gpsimWin32.html"
  Delete "$INSTDIR\doc\gpsim_cvs.html"

  Delete "$INSTDIR\bin\asprintf.dll"
  Delete "$INSTDIR\bin\charset.dll"
  Delete "$INSTDIR\bin\gpsim.exe"
  Delete "$INSTDIR\bin\iconv.dll"
  Delete "$INSTDIR\bin\intl.dll"
  Delete "$INSTDIR\bin\libatk-1.0-0.dll"
  Delete "$INSTDIR\bin\libgdk_pixbuf-2.0-0.dll"
  Delete "$INSTDIR\bin\libgdk-win32-2.0-0.dll"
  Delete "$INSTDIR\bin\libglib-2.0-0.dll"
  Delete "$INSTDIR\bin\libgmodule-2.0-0.dll"
  Delete "$INSTDIR\bin\libgobject-2.0-0.dll"
  Delete "$INSTDIR\bin\libgthread-2.0-0.dll"
  Delete "$INSTDIR\bin\libgtk-win32-2.0-0.dll"
  Delete "$INSTDIR\bin\libiconv-2.dll"
  Delete "$INSTDIR\bin\libintl-2.dll"
  Delete "$INSTDIR\bin\libpango-1.0-0.dll"
  Delete "$INSTDIR\bin\libpangoft2-1.0-0.dll"
  Delete "$INSTDIR\bin\libpangowin32-1.0-0.dll"
  Delete "$INSTDIR\bin\msvcp71.dll"
  Delete "$INSTDIR\bin\msvcr71.dll"
  Delete "$INSTDIR\bin\popt1.dll"
  Delete "$INSTDIR\bin\readline.dll"
  Delete "$INSTDIR\bin\pthreadVC2.dll"

  Delete "$SMPROGRAMS\gpsim\Uninstall.lnk"
  Delete "$SMPROGRAMS\gpsim\Website.lnk"
  Delete "$DESKTOP\gpsim.lnk"
  Delete "$SMPROGRAMS\gpsim\gpsim.lnk"

  Delete "$INSTDIR\include\gpsim\*.h"

  RMDir "$SMPROGRAMS\gpsim"

  RMDir "$INSTDIR\lib\gpsim"
  RMDir "$INSTDIR\lib\pango\1.4.0\modules"
  RMDir "$INSTDIR\lib\pango\1.4.0"
  RMDir "$INSTDIR\lib\pango"
  RMDir "$INSTDIR\lib\gtk-2.0\2.4.0\loaders"
  RMDir "$INSTDIR\lib\gtk-2.0\2.4.0\immodules"
  RMDir "$INSTDIR\lib\gtk-2.0\2.4.0"
  RMDir "$INSTDIR\lib\gtk-2.0"
  RMDir "$INSTDIR\lib"
  RMDir "$INSTDIR\etc\pango"
  RMDir "$INSTDIR\etc\gtk-2.0"
  RMDir "$INSTDIR\etc"
  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR\modules"

  RMDir "$INSTDIR\examples\12bit"
  RMDir "$INSTDIR\examples\14bit"
  RMDir "$INSTDIR\examples\16bit"
  RMDir "$INSTDIR\examples\modules\led_test"
  RMDir "$INSTDIR\examples\modules\logic_test"
  RMDir "$INSTDIR\examples\modules\mod_test"
  RMDir "$INSTDIR\examples\modules\paraface_test"
  RMDir "$INSTDIR\examples\modules\usart_test"
  RMDir "$INSTDIR\examples\modules"
  RMDir "$INSTDIR\examples\projects\digital_stim"
  RMDir "$INSTDIR\examples\projects\p16f628_test"
  RMDir "$INSTDIR\examples\projects\stack_test"
  RMDir "$INSTDIR\examples\projects"
  RMDir "$INSTDIR\examples\scripts"
  RMDir "$INSTDIR\examples"
  
  RMDir "$INSTDIR\extras\lcd\examples"
  RMDir "$INSTDIR\extras\lcd"
  RMDir "$INSTDIR\extras\rs232-gen\example"
  RMDir "$INSTDIR\extras\rs232-gen"
  RMDir "$INSTDIR\extras"

  RMDir "$INSTDIR\include\gpsim"
  RMDir "$INSTDIR\include"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Functions                                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!verbose 4

; CreateBatFile - Create gpsim.bat.

Function CreateBatFile
  Push $0

  FileOpen $0 "$INSTDIR\${PRODUCT_NAME}.bat$\r$\n" w
  FileWrite $0 "@echo off$\r$\n"
  FileWrite $0 "set PATH=$INSTDIR\bin;%PATH$\r$\n"
  FileWrite $0 "$\"$INSTDIR\bin\${PRODUCT_NAME}$\"$\r$\n"
  FileClose $0

  Pop $0
FunctionEnd
