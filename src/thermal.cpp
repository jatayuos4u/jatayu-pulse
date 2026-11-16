#include "thermal.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace {
 QByteArray readBytes(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) return {};
  return f.read(4096).trimmed();
 }
 QString readText(const QString &path) { return QString::fromUtf8(readBytes(path)).trimmed(); }
}
namespace Thermal {
 bool parseMilliCelsius(const QByteArray &raw, double &result) {
  const QByteArray value=raw.trimmed();
  static const QRegularExpression integer(QStringLiteral("^[+-]?[0-9]{1,12}$"));
  if (!integer.match(QString::fromLatin1(value)).hasMatch()) return false;
  bool ok=false;
  const qint64 milli=value.toLongLong(&ok);
  if (!ok || milli < -50000 || milli > 250000) return false;
  result=double(milli)/1000.0;
  return true;
 }
 QVector<ThermalReading> discover(const QString &root) {
  QVector<ThermalReading> out;
  QDir base(root);
  const auto devices=base.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
  static const QRegularExpression inputPattern(QStringLiteral("^temp([0-9]+)_input$"));
  int deviceCount=0;
 for (const QFileInfo &deviceInfo: devices) {
  if(++deviceCount > 256) break;
   const QString path=deviceInfo.absoluteFilePath();
   const QString chip=readText(path+"/name").isEmpty()?deviceInfo.fileName():readText(path+"/name");
   QDir dev(path);
   int sensorCount=0;
   for (const QString &file:dev.entryList({QStringLiteral("temp*_input")},QDir::Files,QDir::Name)) {
    if(++sensorCount > 128 || out.size() >= 2048) break;
    const auto match=inputPattern.match(file);
    if (!match.hasMatch()) continue;
    double current=0;
    if (!parseMilliCelsius(readBytes(path+"/"+file),current)) continue;
    const QString number=match.captured(1);
    ThermalReading r;
    r.id=deviceInfo.fileName()+"/"+file;
    r.device=chip;
    r.label=readText(path+"/temp"+number+"_label");
    if (r.label.isEmpty()) r.label=QStringLiteral("Sensor %1").arg(number);
    r.celsius=current;
    double threshold=0;
    if (parseMilliCelsius(readBytes(path+"/temp"+number+"_max"),threshold)) {
     r.hasHigh=true; r.highCelsius=threshold;
    }
    if (parseMilliCelsius(readBytes(path+"/temp"+number+"_crit"),threshold)) {
     r.hasCritical=true; r.criticalCelsius=threshold;
    }
    out.append(r);
   }
  }
  std::sort(out.begin(),out.end(),[](const ThermalReading &a,const ThermalReading &b){return a.id<b.id;});
  return out;
 }
 QString status(const ThermalReading &r) {
  if (r.hasCritical && r.celsius>=r.criticalCelsius) return QStringLiteral("Critical");
  if (r.hasHigh && r.celsius>=r.highCelsius) return QStringLiteral("High");
  return r.hasHigh||r.hasCritical?QStringLiteral("Below reported limits"):QStringLiteral("No threshold reported");
 }
}
