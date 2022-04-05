#pragma once

#include <string>

#include "base/files/file_path.h"
#include "build/build_config.h"
#include "client/crash_report_database.h"
#include "util/misc/uuid.h"

namespace crashpad {

class CrashReportReportTool {
 public:
  struct Options {
    base::FilePath tool;
  };

  CrashReportReportTool(CrashReportDatabase* database, const Options& options);

  CrashReportReportTool(const CrashReportReportTool&) = delete;
  CrashReportReportTool& operator=(const CrashReportReportTool&) = delete;

  ~CrashReportReportTool();

  void ReportPending(bool simulation, const UUID& report_uuid);

 private:
  const Options options_;
  CrashReportDatabase* database_;  // weak
};

}  // namespace crashpad
