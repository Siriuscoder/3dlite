/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_dbg.h>

#if defined(PLATFORM_Windows) && defined(_DEBUG)
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"DbgHelp.lib")

static LONG WINAPI WinDbgUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    HANDLE hFile = CreateFile("core.dmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    MINIDUMP_EXCEPTION_INFORMATION aMiniDumpInfo;
    aMiniDumpInfo.ThreadId = GetCurrentThreadId();
    aMiniDumpInfo.ExceptionPointers = ExceptionInfo;
    aMiniDumpInfo.ClientPointers = TRUE;

    MiniDumpWriteDump(GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        (MINIDUMP_TYPE) (MiniDumpWithFullMemory|MiniDumpWithHandleData),
        &aMiniDumpInfo,
        NULL,
        NULL);

    CloseHandle(hFile);

    return EXCEPTION_EXECUTE_HANDLER;
}

void lite3d_dbg_enable_coredump(void)
{
    SetUnhandledExceptionFilter(&WinDbgUnhandledExceptionFilter);
}

#else

void lite3d_dbg_enable_coredump(void)
{}

#endif