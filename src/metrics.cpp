#include "metrics.h"
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <limits>

namespace {
QString readFile(const QString &path) { QFile f(path); if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {}; return QString::fromUtf8(f.read(256 * 1024)); }
}
namespace Metrics {
CpuTimes parseCpu(const QString &line) {
 const auto parts=line.simplified().split(' ');
 CpuTimes t;
 if (parts.size()<5 || parts[0]!="cpu") return t;
 QVector<quint64> nums;
 for(int i=1;i<parts.size();++i) { bool ok=false; auto n=parts[i].toULongLong(&ok); if(!ok) return {}; nums.append(n); t.total+=n; }
 t.idle=nums.value(3)+nums.value(4); return t;
}
double cpuPercent(CpuTimes before, CpuTimes after) {
 if(after.total<=before.total || after.idle<before.idle) return 0;
 const auto total=after.total-before.total, idle=after.idle-before.idle;
 return std::clamp(100.0*(1.0-double(idle)/double(total)),0.0,100.0);
}
MemoryInfo parseMemory(const QString &text) {
 MemoryInfo m;
 for(const auto &line:text.split('\n')) {
  const auto p=line.split(':'); if(p.size()!=2) continue;
  bool ok=false; quint64 v=p[1].simplified().section(' ',0,0).toULongLong(&ok); if(!ok) continue;
  if(p[0]=="MemTotal") m.totalKiB=v;
  else if(p[0]=="MemAvailable") m.availableKiB=v;
  else if(p[0]=="SwapTotal") m.swapTotalKiB=v;
  else if(p[0]=="SwapFree") m.swapFreeKiB=v;
 }
 return m;
}
NetworkTotals parseNetwork(const QString &text) {
 NetworkTotals n;
 for(const auto &line:text.split('\n')) {
  const int colon=line.indexOf(':'); if(colon<0) continue;
  const QString name=line.left(colon).trimmed(); if(name=="lo") continue;
  const auto fields=line.mid(colon+1).simplified().split(' ');
  if(fields.size()<16) continue;
  bool ok1=false,ok2=false;
  const auto rx=fields[0].toULongLong(&ok1),tx=fields[8].toULongLong(&ok2);
  if(ok1&&ok2 && rx<=std::numeric_limits<quint64>::max()-n.rx && tx<=std::numeric_limits<quint64>::max()-n.tx) {n.rx+=rx;n.tx+=tx;}
 }
 return n;
}
CpuTimes readCpu() {return parseCpu(readFile("/proc/stat").section('\n',0,0));}
MemoryInfo readMemory() {return parseMemory(readFile("/proc/meminfo"));}
NetworkTotals readNetwork() {return parseNetwork(readFile("/proc/net/dev"));}
QString uptime() {bool ok=false; const double seconds=readFile("/proc/uptime").section(' ',0,0).toDouble(&ok); if(!ok) return "—"; const auto s=quint64(seconds); return QString("%1d %2h %3m").arg(s/86400).arg((s/3600)%24).arg((s/60)%60);}
QVector<ProcessInfo> processes(int maxCount) {
 QVector<ProcessInfo> out;
 QDir proc("/proc");
 int scanned = 0;
 for(const auto &entry:proc.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
  if(++scanned > 16384) break;
  bool ok=false; int pid=entry.toInt(&ok); if(!ok) continue;
  const QString status=readFile(QString("/proc/%1/status").arg(pid)); if(status.isEmpty()) continue;
  ProcessInfo p; p.pid=pid;
  for(const auto &line:status.split('\n')) {
   if(line.startsWith("Name:")) p.name=line.mid(5).trimmed();
   else if(line.startsWith("VmRSS:")) p.rssKiB=line.mid(6).simplified().section(' ',0,0).toULongLong();
  }
  if(!p.name.isEmpty()) out.append(p);
 }
 std::sort(out.begin(),out.end(),[](const auto &a,const auto &b){return a.rssKiB>b.rssKiB;});
 if(maxCount>=0 && out.size()>maxCount) out.resize(maxCount);
 return out;
}
}
