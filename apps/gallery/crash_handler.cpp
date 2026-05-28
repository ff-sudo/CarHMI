#include "crash_handler.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // prevent windows.h from defining min/max macros
#endif
#include <windows.h>
#include <minidumpapiset.h>  // MINIDUMP_EXCEPTION_INFORMATION, MINIDUMP_TYPE, MiniDumpNormal

typedef BOOL (WINAPI *MiniDumpWriteDump_t)(
    HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, const MINIDUMP_EXCEPTION_INFORMATION*,
    const void*, const void*);

static std::string g_exeDir;

static LONG WINAPI SehHandler(EXCEPTION_POINTERS* info) {
    // Load dbghelp dynamically to avoid imagehlp.h header conflicts
    HMODULE dbghelp = LoadLibraryA("dbghelp.dll");
    MiniDumpWriteDump_t pMiniDumpWriteDump = nullptr;
    if (dbghelp)
        pMiniDumpWriteDump = (MiniDumpWriteDump_t)GetProcAddress(dbghelp, "MiniDumpWriteDump");

    // 1. Write minidump next to exe
    SYSTEMTIME st;
    GetLocalTime(&st);
    char filename[MAX_PATH];
    snprintf(filename, sizeof(filename), "%s\\crash_%04d%02d%02d_%02d%02d%02d.dmp",
             g_exeDir.c_str(),
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);

    if (pMiniDumpWriteDump) {
        HANDLE f = CreateFileA(filename, GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (f != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ThreadId          = GetCurrentThreadId();
            dumpInfo.ExceptionPointers = info;
            dumpInfo.ClientPointers    = FALSE;
            pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), f,
                              MiniDumpNormal, &dumpInfo, nullptr, nullptr);
            CloseHandle(f);
            fprintf(stderr, "\n*** CRASH *** Minidump: %s\n", filename);
        }
    }

    // 2. Write crash info text log
    char logPath[MAX_PATH];
    snprintf(logPath, sizeof(logPath), "%s\\crash.log", g_exeDir.c_str());
    FILE* log = fopen(logPath, "a");
    if (log) {
        time_t now = time(nullptr);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log, "\n=== CRASH %s ===\n", timebuf);
        fprintf(log, "Exception: code=0x%08lX, addr=%p\n",
                info->ExceptionRecord->ExceptionCode,
                info->ExceptionRecord->ExceptionAddress);
        fclose(log);
    }

    fflush(stderr);
    if (dbghelp) FreeLibrary(dbghelp);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif  // _WIN32

// backward-cpp for cross-platform stack trace to stderr
#ifdef CARHMI_USE_BACKWARD
#include <backward.hpp>
#endif

namespace CarHMI::Gallery {

void InstallCrashHandler() {
#ifdef _WIN32
    // Resolve exe directory for absolute log paths
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, sizeof(exePath));
    g_exeDir = exePath;
    size_t lastSlash = g_exeDir.rfind('\\');
    if (lastSlash != std::string::npos)
        g_exeDir.resize(lastSlash);

    // Windows: minidump + crash text log (uses dynamic loading, no dbghelp.h needed)
    SetUnhandledExceptionFilter(SehHandler);
#endif

#ifdef CARHMI_USE_BACKWARD
    // Cross-platform stack trace to stderr
    static backward::SignalHandling sh;
#endif
}

const std::string& GetExeDirectory() {
#ifdef _WIN32
    return g_exeDir;
#else
    static const std::string empty;
    return empty;
#endif
}

} // namespace CarHMI::Gallery
