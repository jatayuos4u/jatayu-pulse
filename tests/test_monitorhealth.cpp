#include <QtTest>
#include <limits>
#include "monitorhealth.h"
class TestMonitorHealth : public QObject {
 Q_OBJECT
private slots:
 void noMemory(){QCOMPARE(MonitorHealth::usagePercent(0,0),0.0);}
 void noSwap(){QCOMPARE(MonitorHealth::usagePercent(0,0),0.0);}
 void availableOverTotal(){QCOMPARE(MonitorHealth::usagePercent(100,200),0.0);}
 void halfUsed(){QCOMPARE(MonitorHealth::usagePercent(100,50),50.0);}
 void resetCounter(){QCOMPARE(MonitorHealth::rateKiB(4096,100,1.0),0.0);}
 void zeroInterval(){QCOMPARE(MonitorHealth::rateKiB(0,4096,0.0),0.0);}
 void negativeInterval(){QCOMPARE(MonitorHealth::rateKiB(0,4096,-1.0),0.0);}
 void hugeInterval(){QCOMPARE(MonitorHealth::rateKiB(0,4096,1000.0),0.0);}
 void normalRate(){QCOMPARE(MonitorHealth::rateKiB(0,2048,2.0),1.0);}
};
QTEST_GUILESS_MAIN(TestMonitorHealth)
#include "test_monitorhealth.moc"
