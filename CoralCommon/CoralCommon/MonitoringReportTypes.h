// -*- C++ -*-
// $Id: MonitoringReportTypes.h,v 1.2 2011/03/22 11:22:33 avalassi Exp $
#ifndef CORAL_MONITOR_REPORT_TYPES_H
#define CORAL_MONITOR_REPORT_TYPES_H 1

namespace coral
{
  namespace monitor
  {
    enum ReportType
      {
        Csv = 1,
        Xml,
        SQLite
      };
  }
}

#endif // CORAL_MONITOR_REPORT_TYPES_H
