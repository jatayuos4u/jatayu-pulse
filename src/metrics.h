#pragma once
#include <QString>
#include <QVector>
#include <QtGlobal>

struct CpuTimes { quint64 idle=0,total=0; };
struct MemoryInfo { quint64 totalKiB=0, availableKiB=0, swapTotalKiB=0, swapFreeKiB=0; };
struct NetworkTotals { quint64 rx=0, tx=0; };
struct ProcessInfo { int pid=0; QString name; quint64 rssKiB=0; };
namespace Metrics {
 CpuTimes parseCpu(const QString &line);
 double cpuPercent(CpuTimes before, CpuTimes after);
 MemoryInfo parseMemory(const QString &text);
 NetworkTotals parseNetwork(const QString &text);
 QVector<ProcessInfo> processes(int maxCount=150);
 CpuTimes readCpu();
 MemoryInfo readMemory();
 NetworkTotals readNetwork();
 QString uptime();
}
