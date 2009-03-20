# Microsoft Developer Studio Project File - Name="Napl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Napl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Napl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Napl.mak" CFG="Napl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Napl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Napl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Sounds/libs/Napl", TBAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Napl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "D:\program files\intel\plsuite\Include" /I "..\..\include\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Napl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include\\" /I "D:\program files\intel\plsuite\Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Napl - Win32 Release"
# Name "Napl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aiff.cpp
# End Source File
# Begin Source File

SOURCE=.\Filefact.cpp
# End Source File
# Begin Source File

SOURCE=.\IEEE754.cpp
# End Source File
# Begin Source File

SOURCE=.\objfact.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\wav.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Aiff.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\architec.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\config_linux.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Config_old.h
# End Source File
# Begin Source File

SOURCE=.\Convendi.h
# End Source File
# Begin Source File

SOURCE=.\Delay.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Filefact.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Filetype.h
# End Source File
# Begin Source File

SOURCE=.\Fourier.h
# End Source File
# Begin Source File

SOURCE=.\Ieee754.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\ms_config.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\napl.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\objfact.h
# End Source File
# Begin Source File

SOURCE=.\processor.h
# End Source File
# Begin Source File

SOURCE=.\Resample.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Samplebl.h
# End Source File
# Begin Source File

SOURCE=.\Samplety.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\simpops.h
# End Source File
# Begin Source File

SOURCE=.\Smputils.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\stereomaker.h
# End Source File
# Begin Source File

SOURCE=.\Wav.h
# End Source File
# Begin Source File

SOURCE=.\xfade.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
