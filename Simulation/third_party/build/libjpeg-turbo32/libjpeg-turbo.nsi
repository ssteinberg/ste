!include x64.nsh
Name "libjpeg-turbo SDK for Visual C++"
OutFile "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}libjpeg-turbo-1.5.2-vc.exe"
InstallDir ..\..\

SetCompressor bzip2

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "libjpeg-turbo SDK for Visual C++ (required)"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif
	SectionIn RO
!ifdef GCC
	IfFileExists $SYSDIR/libturbojpeg.dll exists 0
!else
	IfFileExists $SYSDIR/turbojpeg.dll exists 0
!endif
	goto notexists
	exists:
!ifdef GCC
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ is already installed.  Please uninstall it first."
!else
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ or the TurboJPEG SDK is already installed.  Please uninstall it first."
!endif
	quit

	notexists:
	SetOutPath $SYSDIR
!ifdef GCC
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\libturbojpeg.dll"
!else
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}turbojpeg.dll"
!endif
	SetOutPath $INSTDIR\bin
!ifdef GCC
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\libturbojpeg.dll"
!else
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}turbojpeg.dll"
!endif
!ifdef GCC
	File "/oname=libjpeg-62.dll" "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\libjpeg-*.dll"
!else
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\${BUILDDIR}jpeg62.dll"
!endif
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\${BUILDDIR}cjpeg.exe"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\${BUILDDIR}djpeg.exe"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\${BUILDDIR}jpegtran.exe"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}tjbench.exe"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}rdjpgcom.exe"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}wrjpgcom.exe"
	SetOutPath $INSTDIR\lib
!ifdef GCC
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\libturbojpeg.dll.a"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\libturbojpeg.a"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\libjpeg.dll.a"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\libjpeg.a"
!else
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}turbojpeg.lib"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}turbojpeg-static.lib"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\sharedlib\${BUILDDIR}jpeg.lib"
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\${BUILDDIR}jpeg-static.lib"
!endif
!ifdef JAVA
	SetOutPath $INSTDIR\classes
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\java\${BUILDDIR}turbojpeg.jar"
!endif
	SetOutPath $INSTDIR\include
	File "D:/src/git/StE/Simulation/third_party/build/libjpeg-turbo32\jconfig.h"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\jerror.h"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\jmorecfg.h"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\jpeglib.h"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\turbojpeg.h"
	SetOutPath $INSTDIR\doc
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\README.ijg"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\README.md"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\LICENSE.md"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\example.c"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\libjpeg.txt"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\structure.txt"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\usage.txt"
	File "D:/src/git/StE/Simulation/third_party/packages/libjpeg-turbo\wizard.txt"

	WriteRegStr HKLM "SOFTWARE\libjpeg-turbo 1.5.2" "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.5.2" "DisplayName" "libjpeg-turbo SDK v1.5.2 for Visual C++"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.5.2" "UninstallString" '"$INSTDIR\uninstall_1.5.2.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.5.2" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.5.2" "NoRepair" 1
	WriteUninstaller "uninstall_1.5.2.exe"
SectionEnd

Section "Uninstall"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif

	SetShellVarContext all

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo 1.5.2"
	DeleteRegKey HKLM "SOFTWARE\libjpeg-turbo 1.5.2"

!ifdef GCC
	Delete $INSTDIR\bin\libjpeg-62.dll
	Delete $INSTDIR\bin\libturbojpeg.dll
	Delete $SYSDIR\libturbojpeg.dll
	Delete $INSTDIR\lib\libturbojpeg.dll.a"
	Delete $INSTDIR\lib\libturbojpeg.a"
	Delete $INSTDIR\lib\libjpeg.dll.a"
	Delete $INSTDIR\lib\libjpeg.a"
!else
	Delete $INSTDIR\bin\jpeg62.dll
	Delete $INSTDIR\bin\turbojpeg.dll
	Delete $SYSDIR\turbojpeg.dll
	Delete $INSTDIR\lib\jpeg.lib
	Delete $INSTDIR\lib\jpeg-static.lib
	Delete $INSTDIR\lib\turbojpeg.lib
	Delete $INSTDIR\lib\turbojpeg-static.lib
!endif
!ifdef JAVA
	Delete $INSTDIR\classes\turbojpeg.jar
!endif
	Delete $INSTDIR\bin\cjpeg.exe
	Delete $INSTDIR\bin\djpeg.exe
	Delete $INSTDIR\bin\jpegtran.exe
	Delete $INSTDIR\bin\tjbench.exe
	Delete $INSTDIR\bin\rdjpgcom.exe
	Delete $INSTDIR\bin\wrjpgcom.exe
	Delete $INSTDIR\include\jconfig.h"
	Delete $INSTDIR\include\jerror.h"
	Delete $INSTDIR\include\jmorecfg.h"
	Delete $INSTDIR\include\jpeglib.h"
	Delete $INSTDIR\include\turbojpeg.h"
	Delete $INSTDIR\uninstall_1.5.2.exe
	Delete $INSTDIR\doc\README.ijg
	Delete $INSTDIR\doc\README.md
	Delete $INSTDIR\doc\LICENSE.md
	Delete $INSTDIR\doc\example.c
	Delete $INSTDIR\doc\libjpeg.txt
	Delete $INSTDIR\doc\structure.txt
	Delete $INSTDIR\doc\usage.txt
	Delete $INSTDIR\doc\wizard.txt

	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\doc"
!ifdef JAVA
	RMDir "$INSTDIR\classes"
!endif
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"

SectionEnd
