#pragma once
#include <QString>
#include <QVector>
#include <QByteArray>

struct ThermalReading {
 QString id;
 QString device;
 QString label;
 double celsius=0;
 double highCelsius=0;
 double criticalCelsius=0;
 bool hasHigh=false;
 bool hasCritical=false;
};
namespace Thermal {
 // Kernel hwmon values are millidegrees Celsius; malformed values are rejected.
 bool parseMilliCelsius(const QByteArray &raw, double &result);
 QVector<ThermalReading> discover(const QString &hwmonRoot=QStringLiteral("/sys/class/hwmon"));
 QString status(const ThermalReading &reading);
}
