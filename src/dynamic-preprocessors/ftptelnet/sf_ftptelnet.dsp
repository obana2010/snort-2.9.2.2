# Microsoft Developer Studio Project File - Name="sf_ftptelnet" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sf_ftptelnet - Win32 IPv6 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sf_ftptelnet.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sf_ftptelnet.mak" CFG="sf_ftptelnet - Win32 IPv6 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sf_ftptelnet - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_ftptelnet - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_ftptelnet - Win32 IPv6 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_ftptelnet - Win32 IPv6 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sf_ftptelnet - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SMTP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\libs" /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "ENABLE_PAF" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FD /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "sf_ftptelnet - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SMTP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\libs" /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "_DEBUG" /D "DEBUG" /D "ENABLE_PAF" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "sf_ftptelnet - Win32 IPv6 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sf_ftptelnet___Win32_IPv6_Release"
# PROP BASE Intermediate_Dir "sf_ftptelnet___Win32_IPv6_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "IPv6_Release"
# PROP Intermediate_Dir "IPv6_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "NDEBUG" /D "SF_SMTP_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SNORT_PREPROC_DLL" /D "HAVE_CONFIG_H" /D "DYNAMIC_PLUGIN" /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\libs" /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "SUP_IP6" /D "ENABLE_PAF" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FD /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "sf_ftptelnet - Win32 IPv6 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sf_ftptelnet___Win32_IPv6_Debug"
# PROP BASE Intermediate_Dir "sf_ftptelnet___Win32_IPv6_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "IPv6_Debug"
# PROP Intermediate_Dir "IPv6_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\win32\Win32-Includes" /I "..\..\\" /I ".\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_SNORT_PREPROC_DLL" /D "HAVE_CONFIG_H" /D "DYNAMIC_PLUGIN" /YX /FD /GZ /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\libs" /I "..\include" /I "..\..\win32\Win32-Includes" /I ".\\" /I "..\..\win32\Win32-Includes\WinPCAP" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "SUP_IP6" /D "_DEBUG" /D "DEBUG" /D "ENABLE_PAF" /D "SF_SNORT_PREPROC_DLL" /D "DYNAMIC_PLUGIN" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sf_ftptelnet - Win32 Release"
# Name "sf_ftptelnet - Win32 Debug"
# Name "sf_ftptelnet - Win32 IPv6 Release"
# Name "sf_ftptelnet - Win32 IPv6 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ftp_bounce_lookup.c
# End Source File
# Begin Source File

SOURCE=.\ftp_cmd_lookup.c
# End Source File
# Begin Source File

SOURCE=.\ftpp_eo_log.c
# End Source File
# Begin Source File

SOURCE=.\ftpp_si.c
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_client_lookup.c
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_config.c
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_server_lookup.c
# End Source File
# Begin Source File

SOURCE=.\hi_util_kmap.c
# End Source File
# Begin Source File

SOURCE=.\hi_util_xmalloc.c
# End Source File
# Begin Source File

SOURCE=..\include\inet_aton.c
# End Source File
# Begin Source File

SOURCE=..\include\inet_pton.c
# End Source File
# Begin Source File

SOURCE=.\pp_ftp.c
# End Source File
# Begin Source File

SOURCE=.\pp_telnet.c
# End Source File
# Begin Source File

SOURCE=..\include\sf_dynamic_preproc_lib.c
# End Source File
# Begin Source File

SOURCE=..\include\sf_ip.c
# End Source File
# Begin Source File

SOURCE=..\include\sfPolicyUserData.c
# End Source File
# Begin Source File

SOURCE=..\include\sfrt.c
# End Source File
# Begin Source File

SOURCE=..\include\sfrt_dir.c
# End Source File
# Begin Source File

SOURCE=.\snort_ftptelnet.c
# End Source File
# Begin Source File

SOURCE=.\spp_ftptelnet.c
# End Source File
# Begin Source File

SOURCE=..\include\strtok_r.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ftp_bounce_lookup.h
# End Source File
# Begin Source File

SOURCE=.\ftp_client.h
# End Source File
# Begin Source File

SOURCE=.\ftp_cmd_lookup.h
# End Source File
# Begin Source File

SOURCE=.\ftp_server.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_eo.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_eo_events.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_eo_log.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_include.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_return_codes.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_si.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_client_lookup.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_config.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_ui_server_lookup.h
# End Source File
# Begin Source File

SOURCE=.\ftpp_util_kmap.h
# End Source File
# Begin Source File

SOURCE=.\hi_util_kmap.h
# End Source File
# Begin Source File

SOURCE=.\hi_util_xmalloc.h
# End Source File
# Begin Source File

SOURCE=.\pp_ftp.h
# End Source File
# Begin Source File

SOURCE=.\pp_telnet.h
# End Source File
# Begin Source File

SOURCE=.\sf_preproc_info.h
# End Source File
# Begin Source File

SOURCE=.\snort_ftptelnet.h
# End Source File
# Begin Source File

SOURCE=.\spp_ftptelnet.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
