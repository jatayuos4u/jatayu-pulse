#include <QtTest>
#include "metrics.h"
class TestMetrics:public QObject {
 Q_OBJECT
private slots:
 void cpu() {auto a=Metrics::parseCpu("cpu  100 0 100 800 0 0 0 0 0 0");auto b=Metrics::parseCpu("cpu  150 0 150 900 0 0 0 0 0 0");QCOMPARE(Metrics::cpuPercent(a,b),50.0);}
 void cpuNoDelta() {CpuTimes a{5,10};QCOMPARE(Metrics::cpuPercent(a,a),0.0);}
 void memory() {auto m=Metrics::parseMemory("MemTotal: 8192000 kB\nMemAvailable: 4096000 kB\nSwapTotal: 1024 kB\nSwapFree: 512 kB\n");QCOMPARE(m.totalKiB,quint64(8192000));QCOMPARE(m.availableKiB,quint64(4096000));}
 void network() {auto n=Metrics::parseNetwork(" lo: 100 0 0 0 0 0 0 0 100 0 0 0 0 0 0 0\n eth0: 2048 0 0 0 0 0 0 0 4096 0 0 0 0 0 0 0\n");QCOMPARE(n.rx,quint64(2048));QCOMPARE(n.tx,quint64(4096));}
};
QTEST_GUILESS_MAIN(TestMetrics)
#include "test_metrics.moc"
