#include "handler/crash_report_report_tool.h"

#include "base/logging.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_APPLE)
#include "util/posix/spawn_subprocess.h"
#elif BUILDFLAG(IS_WIN)
#include <sstream>

#include <windows.h>
#endif  // BUILDFLAG(IS_APPLE)

namespace crashpad {

namespace {

void RunTool(bool simulation,
             const base::FilePath& tool,
             const base::FilePath& dump_path) {
#if BUILDFLAG(IS_APPLE)
  std::vector<std::string> argv;

  argv.reserve(4);
  argv.push_back(tool.value());

  if (simulation) {
    argv.push_back("-S");
  }

  argv.push_back("--dump");
  argv.push_back(dump_path.value());

  if (!SpawnSubprocess(argv, nullptr, -1, false, nullptr)) {
    LOG(ERROR) << "SpawnSubprocess";
  }
#elif BUILDFLAG(IS_WIN)
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  ZeroMemory(&pi, sizeof(pi));

  std::wstringstream ss;

  ss << L"\"" << tool.value().c_str() << L"\"";
  if (simulation) {
    ss << " -S";
  }
  ss << " --dump" << L" \"" << dump_path.value().c_str() << L"\"";

  std::wstring args = ss.str();

  if (!CreateProcessW(nullptr,
                      args.data(),
                      nullptr,
                      nullptr,
                      FALSE,
                      0,
                      nullptr,
                      nullptr,
                      &si,
                      &pi)) {
    LOG(ERROR) << "CreateProcess";
    return;
  }

  if (!CloseHandle(pi.hThread)) {
    LOG(ERROR) << "CloseHandle thread";
  }
  if (!CloseHandle(pi.hProcess)) {
    LOG(ERROR) << "CloseHandle process";
  }
#endif
}

}  // namespace

CrashReportReportTool::CrashReportReportTool(CrashReportDatabase* database,
                                             const Options& options)
    : options_(options), database_(database) {}

CrashReportReportTool::~CrashReportReportTool() {}

void CrashReportReportTool::ReportPending(bool simulation,
                                          const UUID& report_uuid) {
  CrashReportDatabase::Report report;

  if (database_->LookUpCrashReport(report_uuid, &report) !=
      CrashReportDatabase::kNoError) {
    LOG(ERROR) << "Report not found";
    return;
  }

  if (report.file_path.empty()) {
    LOG(ERROR) << "Invalid dump path";
    return;
  }

  RunTool(simulation, options_.tool, report.file_path);
}

}  // namespace crashpad
