/* Copyright 2022 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

/*
A regression test suite. The idea is to add tests for bugs we fix that
are too time consuming to be part of unit tests. The tests can rely
on presence of shared test files.

Note: because it can be run as both release and debug, we can't use
assert() or ReportIf() but ReportIf().

To write new regression test:
- add a file src/regress/Regress${NN}.cpp with Regress${NN} function
- #include "Regress${NN}.cpp" right before RunTest() function
- call Regress${NN} function from RunTests()
*/

#include "utils/BaseUtil.h"
#include "utils/ScopedWin.h"
#include "utils/WinDynCalls.h"
#include "utils/Archive.h"
#include "utils/DbgHelpDyn.h"
#include "utils/FileUtil.h"
#include "utils/GuessFileType.h"
#include "utils/GdiPlusUtil.h"
#include "utils/HtmlParserLookup.h"
#include "mui/Mui.h"
#include "utils/WinUtil.h"

#include "wingui/UIModels.h"

#include "Settings.h"
#include "DocProperties.h"
#include "DocController.h"
#include "EngineBase.h"
#include "EbookBase.h"
#include "EbookDoc.h"
#include "HtmlFormatter.h"
#include "EbookFormatter.h"
// For Regress03 (Text Search)
#include "EngineAll.h"
#include "ProgressUpdateUI.h"
#include "TextSelection.h"
#include "TextSearch.h"

static const char* gTestFilesDir;

static const char* TestFilesDir() {
    return gTestFilesDir;
}

static int Usage() {
    printf("regress.exe\n");
    printf("Error: didn't find test files on this computer!\n");
    system("pause");
    return 1;
}

static void printflush(const char* s) {
    printf(s);
    fflush(stdout);
}

/* Auto-detect the location of test files. Ultimately we might add a cmd-line
option to specify this directory, for now just add your location(s) to the list */
static bool FindTestFilesDir() {
    const char* dirsToCheck[] = {"C:\\Documents and Settings\\kkowalczyk\\My Documents\\Google Drive\\Sumatra",
                                 "C:\\Users\\kkowalczyk\\Google Drive\\Sumatra"};
    for (size_t i = 0; i < dimof(dirsToCheck); i++) {
        const char* dir = dirsToCheck[i];
        if (dir::Exists(dir)) {
            gTestFilesDir = dir;
            return true;
        }
    }
    return false;
}

static void VerifyFileExists(const char* filePath) {
    if (!file::Exists(filePath)) {
        printf("File '%s' doesn't exist!\n", filePath);
        system("pause");
        exit(1);
    }
}

static HANDLE gDumpEvent = nullptr;
static HANDLE gDumpThread = nullptr;
static bool gCrashed = false;

static MINIDUMP_EXCEPTION_INFORMATION gMei{0};
static LPTOP_LEVEL_EXCEPTION_FILTER gPrevExceptionFilter = nullptr;

static DWORD WINAPI CrashDumpThread(void*) {
    WaitForSingleObject(gDumpEvent, INFINITE);
    if (!gCrashed)
        return 0;

    printflush("Captain, we've got a crash!\n");
    if (!dbghelp::Initialize(L"", false)) {
        printflush("CrashDumpThread(): dbghelp::Initialize() failed");
        return 0;
    }

    if (!dbghelp::HasSymbols()) {
        printflush("CrashDumpThread(): dbghelp::HasSymbols() is false");
        return 0;
    }

    str::Str s(16 * 1024);
    dbghelp::GetExceptionInfo(s, gMei.ExceptionPointers);
    dbghelp::GetAllThreadsCallstacks(s);
    s.Append("\r\n");
    printflush(s.LendData());
    return 0;
}

static LONG WINAPI CrashDumpExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    if (!exceptionInfo || (EXCEPTION_BREAKPOINT == exceptionInfo->ExceptionRecord->ExceptionCode))
        return EXCEPTION_CONTINUE_SEARCH;

    static bool wasHere = false;
    if (wasHere)
        return EXCEPTION_CONTINUE_SEARCH;
    wasHere = true;
    gCrashed = true;

    gMei.ThreadId = GetCurrentThreadId();
    gMei.ExceptionPointers = exceptionInfo;
    // per msdn (which is backed by my experience), MiniDumpWriteDump() doesn't
    // write callstack for the calling thread correctly. We use msdn-recommended
    // work-around of spinning a thread to do the writing
    SetEvent(gDumpEvent);
    WaitForSingleObject(gDumpThread, INFINITE);

    TerminateProcess(GetCurrentProcess(), 1);
    return EXCEPTION_CONTINUE_SEARCH;
}

static void InstallCrashHandler() {
    gDumpEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!gDumpEvent) {
        printflush("InstallCrashHandler(): CreateEvent() failed\n");
        return;
    }
    gDumpThread = CreateThread(nullptr, 0, CrashDumpThread, nullptr, 0, nullptr);
    if (!gDumpThread) {
        printflush("InstallCrashHandler(): CreateThread() failed\n");
        return;
    }
    gPrevExceptionFilter = SetUnhandledExceptionFilter(CrashDumpExceptionHandler);
}

static void UninstallCrashHandler() {
    if (!gDumpEvent || !gDumpThread)
        return;

    if (gPrevExceptionFilter)
        SetUnhandledExceptionFilter(gPrevExceptionFilter);

    SetEvent(gDumpEvent);
    WaitForSingleObject(gDumpThread, 1000); // 1 sec

    SafeCloseHandle(&gDumpThread);
    SafeCloseHandle(&gDumpEvent);
}

#include "Regress00.cpp"
#include "Regress03.cpp"

static void RunTests() {
    Regress00();
    Regress01();
    Regress02();
    Regress03();
}

int RegressMain() {
    RedirectIOToConsole();

    if (!FindTestFilesDir()) {
        return Usage();
    }

    InstallCrashHandler();
    InitAllCommonControls();
    ScopedGdiPlus gdi;
    mui::Initialize();

    RunTests();

    printflush("All tests completed successfully!\n");
    mui::Destroy();
    UninstallCrashHandler();

    system("pause");
    return 0;
}
