#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>
class ChartWidget : public QWidget {
 Q_OBJECT
public:
 explicit ChartWidget(const QColor &color,QWidget *parent=nullptr);
 void appendValue(double value);
 void setTrafficMode(bool enabled) { trafficMode_=enabled; update(); }
 void setTemperatureMode(bool enabled) { temperatureMode_=enabled; update(); }
protected:
 void paintEvent(QPaintEvent *) override;
private:
 QColor color_;
 bool temperatureMode_=false;
 bool trafficMode_=false;
 double trafficScale_=100.0;
 QVector<double> values_;
};
