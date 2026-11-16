#include <QtTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include "thermal.h"
namespace {
 void writeFile(const QString &name,const QByteArray &value) { QFile f(name);QVERIFY(f.open(QIODevice::WriteOnly));QCOMPARE(f.write(value),value.size()); }
}
class TestThermal:public QObject {
 Q_OBJECT
private slots:
 void parsing() { double c=0;QVERIFY(Thermal::parseMilliCelsius("56000\n",c));QCOMPARE(c,56.0);QVERIFY(Thermal::parseMilliCelsius("-12500",c));QCOMPARE(c,-12.5);QVERIFY(!Thermal::parseMilliCelsius("not-a-number",c));QVERIFY(!Thermal::parseMilliCelsius("9999999",c)); }
 void discovery() {QTemporaryDir tmp;QVERIFY(tmp.isValid());QVERIFY(QDir().mkpath(tmp.path()+"/hwmon0"));const QString dir=tmp.path()+"/hwmon0/";writeFile(dir+"name","coretemp\n");writeFile(dir+"temp1_input","56000\n");writeFile(dir+"temp1_label","Core 0\n");writeFile(dir+"temp1_max","90000\n");writeFile(dir+"temp1_crit","90000\n");writeFile(dir+"temp2_input","bad\n");const auto rows=Thermal::discover(tmp.path());QCOMPARE(rows.size(),1);QCOMPARE(rows[0].device,QString("coretemp"));QCOMPARE(rows[0].label,QString("Core 0"));QCOMPARE(rows[0].celsius,56.0);QVERIFY(rows[0].hasHigh);QCOMPARE(Thermal::status(rows[0]),QString("Below reported limits")); }
 void thresholds() {ThermalReading r;r.celsius=95;r.hasHigh=true;r.highCelsius=90;QCOMPARE(Thermal::status(r),QString("High"));r.hasCritical=true;r.criticalCelsius=95;QCOMPARE(Thermal::status(r),QString("Critical"));}
};
QTEST_GUILESS_MAIN(TestThermal)
#include "test_thermal.moc"
