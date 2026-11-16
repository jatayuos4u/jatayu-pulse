#include <QtTest>
#include "networkrate.h"
class TestNetworkRate:public QObject {
 Q_OBJECT
private slots:
 void normal(){QCOMPARE(NetworkRate::kibPerSecond(1024,3072,2.0),1.0);}
 void counterReset(){QCOMPARE(NetworkRate::kibPerSecond(4096,1024,1.0),0.0);}
 void zeroTime(){QCOMPARE(NetworkRate::kibPerSecond(0,1024,0.0),0.0);}
 void variableInterval(){QCOMPARE(NetworkRate::kibPerSecond(0,3072,3.0),1.0);}
};
QTEST_GUILESS_MAIN(TestNetworkRate)
#include "test_networkrate.moc"
