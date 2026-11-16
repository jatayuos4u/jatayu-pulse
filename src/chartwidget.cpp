#include "chartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>
ChartWidget::ChartWidget(const QColor &color,QWidget *parent):QWidget(parent),color_(color) {setMinimumHeight(105);setMouseTracking(true);}
void ChartWidget::appendValue(double v) {if(!std::isfinite(v))return;values_.append(trafficMode_?std::max(0.0,v):std::clamp(v,0.0,100.0));
 if(values_.size()>60)values_.remove(0,values_.size()-60);
 if(trafficMode_) {
  double peak=0.0;
  for(double value:values_)peak=std::max(peak,value);
  const double base=std::pow(10.0,std::floor(std::log10(std::max(peak,1.0))));
  const double normalized=peak/base;
  trafficScale_=(normalized<=1.0?1.0:normalized<=2.0?2.0:normalized<=5.0?5.0:10.0)*base;
 }
 update();}
void ChartWidget::paintEvent(QPaintEvent *) {
 QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
 const QRectF r=QRectF(rect()).adjusted(trafficMode_?68:39,9,-10,-24);
 if(r.width()<20||r.height()<20)return;
 QFont f=p.font();f.setPixelSize(10);p.setFont(f);
 for(int i=0;i<=4;++i){const double y=r.top()+r.height()*i/4.0;
  p.setPen(QPen(QColor(85,105,130,95),1,Qt::DashLine));p.drawLine(QPointF(r.left(),y),QPointF(r.right(),y));
  p.setPen(QColor("#93a4bd"));p.drawText(QRectF(0,y-9,trafficMode_?63:34,18),Qt::AlignRight|Qt::AlignVCenter,trafficMode_?
    (trafficScale_>=1024.0?QString::number((trafficScale_*(4-i)/4.0)/1024.0,'f',1)+" MiB/s":
    QString::number(trafficScale_*(4-i)/4.0,'f',trafficScale_<10?1:0)+" KiB/s"):
    QString::number(100-25*i)+(temperatureMode_?"°C":"%"));}
 p.setPen(QColor("#93a4bd"));p.drawText(QRectF(r.left(),r.bottom()+3,130,18),Qt::AlignLeft,
  QString::number(values_.size())+" samples");p.drawText(QRectF(r.right()-65,r.bottom()+3,65,18),Qt::AlignRight,"Now");
 if(values_.size()<2)return;
 QPainterPath path;const double step=r.width()/59.0;
 for(int i=0;i<values_.size();++i){const QPointF pt(r.right()-(values_.size()-1-i)*step,r.bottom()-r.height()*std::min(values_[i]/(trafficMode_?trafficScale_:100.0),1.0));if(i==0)path.moveTo(pt);else path.lineTo(pt);}
 QPainterPath area=path;area.lineTo(r.right(),r.bottom());area.lineTo(r.right()-(values_.size()-1)*step,r.bottom());area.closeSubpath();
 QColor shade=color_;shade.setAlpha(35);p.fillPath(area,shade);p.setPen(QPen(color_,2.3));p.drawPath(path);
 const QPointF end(r.right(),r.bottom()-r.height()*std::min(values_.last()/(trafficMode_?trafficScale_:100.0),1.0));p.setBrush(color_);p.drawEllipse(end,3,3);
}
