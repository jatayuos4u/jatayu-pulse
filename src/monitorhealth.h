#pragma once
#include <QtGlobal>
#include <algorithm>
#include <cmath>
namespace MonitorHealth {
inline double usagePercent(quint64 total, quint64 available) {
 if(total==0) return 0.0;
 return 100.0 * double(total-std::min(total,available)) / double(total);
}
inline bool validSampleInterval(double seconds) {
 return std::isfinite(seconds) && seconds >= 0.05 && seconds <= 60.0;
}
inline double rateKiB(quint64 previous, quint64 current, double seconds) {
 if(!validSampleInterval(seconds) || current<previous) return 0.0;
 return double(current-previous)/(1024.0*seconds);
}
}
