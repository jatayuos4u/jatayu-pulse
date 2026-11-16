#pragma once
#include "monitorhealth.h"
namespace NetworkRate {
inline double kibPerSecond(quint64 previous, quint64 current, double seconds) {
 return MonitorHealth::rateKiB(previous,current,seconds);
}
}
