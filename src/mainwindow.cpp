#include "mainwindow.h"
#include "chartwidget.h"
#include "thermal.h"
#include "networkrate.h"
#include "monitorhealth.h"
#include <KLocalizedString>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QSysInfo>
#include <QFile>
#include <QFont>
#include <QStyle>
#include <QAbstractItemView>
#include <QTableWidgetItem>
#include <QStorageInfo>
#include <QDateTime>
#include <QScrollArea>
#include <QProgressBar>
#include <QRegularExpression>
#include <QScreen>
#include <QGuiApplication>
#include <algorithm>
#include <QComboBox>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDesktopServices>
#include <QSettings>
#include <QThread>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QIcon>
#include <QPixmap>
#include <QSignalBlocker>

namespace {
QString friendlyDevice(const QString &raw){
 const auto name=raw.toLower();
 if(name.startsWith("coretemp"))return QStringLiteral("Intel CPU");
 if(name.startsWith("k10temp"))return QStringLiteral("AMD CPU");
 if(name.startsWith("acpitz"))return QStringLiteral("ACPI Thermal Zone");
 if(name.startsWith("soc_dts"))return QStringLiteral("SoC Sensor ")+name.mid(7);
 if(name.startsWith("iwlwifi"))return QStringLiteral("Intel Wi-Fi Adapter");
 if(name.startsWith("amdgpu"))return QStringLiteral("AMD GPU");
 if(name.startsWith("nouveau"))return QStringLiteral("NVIDIA GPU");
 if(name.startsWith("nvme"))return QStringLiteral("NVMe SSD");
 return raw;
}
QString readTextFile(const QString &path){QFile f(path);if(!f.open(QIODevice::ReadOnly|QIODevice::Text))return {};return QString::fromUtf8(f.read(16384)).trimmed();}
QString cpuModel(){const QString data=readTextFile("/proc/cpuinfo");QRegularExpression rx("(?:model name|Hardware|Processor)\\s*:\\s*([^\\n]+)");auto m=rx.match(data);return m.hasMatch()?m.captured(1).trimmed():QStringLiteral("Not reported");}
QString sysDmi(const QString &key){auto value=readTextFile("/sys/class/dmi/id/"+key);return value.isEmpty()?QStringLiteral("Not accessible"):value;}
QString trafficSize(quint64 bytes){
 if(bytes<1024)return QString::number(bytes)+" B";
 if(bytes<1048576)return QString::number(bytes/1024.0,'f',1)+" KiB";
 if(bytes<1073741824)return QString::number(bytes/1048576.0,'f',1)+" MiB";
 return QString::number(bytes/1073741824.0,'f',2)+" GiB";
}
QString interfaceCounter(const QString &name,const QString &counter){
 QFile file("/sys/class/net/"+name+"/statistics/"+counter);
 if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return QStringLiteral("Unavailable");
 bool ok=false;
 const quint64 bytes=QString::fromUtf8(file.read(128)).trimmed().toULongLong(&ok);
 return ok?trafficSize(bytes):QStringLiteral("Unavailable");
}
QString humanSize(qint64 bytes){if(bytes<=0)return QStringLiteral("—");return QString::number(double(bytes)/1073741824.0,'f',1)+QStringLiteral(" GiB");}
void addRows(QTableWidget *t,const QStringList &cells){if(t->rowCount()>=4096)return;int r=t->rowCount();t->insertRow(r);for(int c=0;c<cells.size();++c){auto *item=new QTableWidgetItem(cells[c]);item->setToolTip(cells[c]);t->setItem(r,c,item);}}
void collectBlocks(const QJsonArray &arr,QTableWidget *t,const QString &parent=QString()){
 for(const auto &v:arr){if(t->rowCount()>=4096)return;const auto o=v.toObject();const QString type=o.value("type").toString();const QString name=o.value("name").toString();const QString path=o.value("path").toString();const QString model=o.value("model").toString().trimmed();const QString fs=o.value("fstype").toString();const QString mount=o.value("mountpoint").toString();const QString labelText=o.value("label").toString();const qint64 size=qint64(o.value("size").toDouble());
 const QString detail=type=="disk"?model:(labelText.isEmpty()?fs:(labelText+" · "+fs));
 addRows(t,{name,path,type,humanSize(size),detail,mount.isEmpty()?QStringLiteral("Not mounted"):mount});
 collectBlocks(o.value("children").toArray(),t,name);
 }
}
void refreshBlockDevices(QTableWidget *t){
 // Use the trusted util-linux executable, not an executable supplied via PATH.
 QProcess proc;
 proc.setProgram(QStringLiteral("/usr/bin/lsblk"));
 proc.setArguments({QStringLiteral("-J"),QStringLiteral("-b"),QStringLiteral("-o"),QStringLiteral("NAME,PATH,TYPE,SIZE,FSTYPE,LABEL,MOUNTPOINT,MODEL")});
 proc.setProcessChannelMode(QProcess::SeparateChannels);
 proc.start();
 const bool started=proc.waitForStarted(1000);
 const bool finished=started && proc.waitForFinished(2500);
 if(started && !finished){proc.kill();proc.waitForFinished(1000);}
 QByteArray output;
 if(finished && proc.exitStatus()==QProcess::NormalExit && proc.exitCode()==0)
  output=proc.readAllStandardOutput();
 t->setRowCount(0);
 if(output.isEmpty() || output.size()>1024*1024){
  addRows(t,{QStringLiteral("lsblk unavailable or output limit exceeded"),QString(),QString(),QString(),QString(),QString()});return;
 }
 QJsonParseError err;const auto doc=QJsonDocument::fromJson(output,&err);
 if(err.error==QJsonParseError::NoError && doc.isObject())collectBlocks(doc.object().value("blockdevices").toArray(),t);
 else addRows(t,{QStringLiteral("Invalid lsblk output"),QString(),QString(),QString(),QString(),QString()});
}
QLabel *detailLine(const QString &name,const QString &value){auto *l=new QLabel(QStringLiteral("<b>%1</b>  %2").arg(name.toHtmlEscaped(),value.toHtmlEscaped()));l->setTextFormat(Qt::RichText);l->setWordWrap(true);l->setTextInteractionFlags(Qt::TextSelectableByMouse);return l;}
QLabel *label(const QString &text,const QString &kind="") {
 auto *w=new QLabel(text);if(!kind.isEmpty())w->setProperty("kind",kind);return w;
}
QFrame *panel(QVBoxLayout *&layout) {
 auto *f=new QFrame;f->setObjectName("panel");layout=new QVBoxLayout(f);
 layout->setContentsMargins(12,9,12,9);layout->setSpacing(5);return f;
}
QFrame *metricCard(const QString &title,QLabel *&value,QLabel *&detail,ChartWidget *chart=nullptr) {
 QVBoxLayout *v;auto *f=panel(v);v->addWidget(label(title,"eyebrow"));
 value=label("—","value");value->setMinimumWidth(0);v->addWidget(value);
 detail=label(" ","muted");detail->setWordWrap(true);v->addWidget(detail);
 if(chart){chart->setMinimumHeight(65);v->addWidget(chart,1);}
 return f;
}
QWidget *page(QVBoxLayout *&v,const QString &title,const QString &subtitle) {
 auto *w=new QWidget;v=new QVBoxLayout(w);v->setContentsMargins(18,12,18,12);v->setSpacing(8);
 v->addWidget(label(title,"heading"));v->addWidget(label(subtitle,"muted"));return w;
}
void setupTable(QTableWidget *t) {
 t->verticalHeader()->hide();t->setEditTriggers(QAbstractItemView::NoEditTriggers);
 t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setAlternatingRowColors(true);
 t->horizontalHeader()->setStretchLastSection(true);t->setShowGrid(false);
 t->setSortingEnabled(false);t->setMinimumHeight(300);
}
}
MainWindow::MainWindow(QWidget *parent):QMainWindow(parent) {
 setWindowIcon(QIcon::fromTheme(QStringLiteral("jatayu-pulse"), QIcon(QStringLiteral(":/jatayu-pulse.png"))));
 setWindowTitle(i18n("Jatayu Pulse"));resize(1366,768);setMinimumSize(940,650);
 setStyleSheet(R"(
 QMainWindow {background:#0b1324;color:#f5f8ff;}
 QWidget {font-family:'Noto Sans';font-size:13px;color:#eef5ff;}
 QFrame#sidebar {background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #111b32,stop:1 #14284a);border-right:1px solid #31446a;}
 QFrame#panel {background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1a2944,stop:1 #111d35);border:1px solid #3b5279;border-radius:16px;}
 QFrame#metric {background:#14233c;border:1px solid #3c659a;border-radius:16px;}
 QFrame#metric[accent="blue"] {border:1px solid #00aaff;background:#102b4c;}
 QFrame#metric[accent="green"] {border:1px solid #00dd88;background:#0b3436;}
 QFrame#metric[accent="purple"] {border:1px solid #ad63fa;background:#282046;}
 QFrame#metric[accent="orange"] {border:1px solid #ff9b15;background:#342535;}
 QLabel[kind="brand"] {font-size:20px;font-weight:800;color:#ffffff;}
 QLabel[kind="heading"] {font-size:28px;font-weight:800;color:#ffffff;}
 QLabel[kind="value"] {font-size:25px;font-weight:800;color:#ffffff;}
 QLabel[kind="eyebrow"] {font-size:14px;font-weight:700;color:#e4ecfb;}
 QLabel[kind="muted"] {font-size:12px;color:#b4c5df;}
 QLabel[kind="thermalValue"] {font-size:17px;font-weight:800;color:#ffffff;}
 QLabel[kind="thermalName"] {font-size:11px;color:#d4e6fb;}
 QPushButton[nav="true"] {text-align:left;padding:12px 16px;background:transparent;border:0;border-radius:9px;color:#f0f4ff;font-weight:600;}
 QPushButton[nav="true"]:hover {background:#234775;}
 QPushButton[active="true"] {background:#086cf0;color:#ffffff;border-left:3px solid #36d5ff;}
 QProgressBar {background:#273b5b;border:0;border-radius:6px;height:10px;text-align:center;}
 QProgressBar::chunk {background:#008bff;border-radius:6px;}
 QLineEdit {padding:10px;background:#141e31;border:1px solid #425778;border-radius:8px;}
 QTableWidget {background:#172237;alternate-background-color:#202f48;border:0;selection-background-color:#315d91;gridline-color:#33445f;}
 QHeaderView::section {background:#263956;color:#e6f0ff;padding:9px;border:0;border-bottom:1px solid #394f72;}
 QScrollArea {background:transparent;border:0;}
 QScrollBar:vertical {background:#152239;width:9px;} QScrollBar::handle:vertical {background:#58749d;border-radius:4px;}
 )");;
 auto *root=new QWidget;setCentralWidget(root);auto *outer=new QHBoxLayout(root);outer->setContentsMargins(0,0,0,0);outer->setSpacing(0);
 auto *side=new QFrame;side->setObjectName("sidebar");side->setFixedWidth(245);
 auto *sv=new QVBoxLayout(side);sv->setContentsMargins(14,22,14,18);sv->setSpacing(8);
 auto *brandRow=new QHBoxLayout;auto *brandIcon=new QLabel;brandIcon->setPixmap(windowIcon().pixmap(QSize(40,40)));brandRow->addWidget(brandIcon);auto *brandText=new QVBoxLayout;brandText->addWidget(label("Jatayu Pulse","brand"));brandText->addWidget(label("System Monitor","muted"));brandRow->addLayout(brandText);brandRow->addStretch();sv->addLayout(brandRow);sv->addSpacing(25);
 const QStringList names={i18n("Dashboard"),i18n("Performance"),i18n("Processes"),i18n("Temperatures"),i18n("Storage & Disks"),i18n("Network"),i18n("System Info"),i18n("Settings"),i18n("About")};
 const QStringList icons={"⌂  ","▥  ","☷  ","♧  ","▱  ","◉  ","▣  ","⚙  ","ⓘ  "};
 for(int i=0;i<names.size();++i){auto *b=new QPushButton(icons[i]+names[i]);b->setProperty("nav",true);b->setCursor(Qt::PointingHandCursor);sv->addWidget(b);navButtons_.append(b);connect(b,&QPushButton::clicked,this,[this,i]{selectPage(i);});}
 sv->addStretch();auto *osBrand=label("JatayuOS Udaan 1.0","eyebrow");osBrand->setWordWrap(true);
 sv->addWidget(osBrand);sv->addWidget(label("Debian 13 (Trixie)","muted"));outer->addWidget(side);
 pages_=new QStackedWidget;outer->addWidget(pages_,1);
 // Dashboard: compact resource strip, prominent live charts and hardware section.
 QVBoxLayout *dv;auto *dashboard=page(dv,i18n("System Dashboard"),i18n("Real-time system performance, temperature and hardware health"));
 auto *headerBar=new QHBoxLayout;headerBar->addStretch();healthLabel_=label("●  Monitoring live","eyebrow");healthLabel_->setStyleSheet("color:#3de88c;padding:8px;");headerBar->addWidget(healthLabel_);uptimeLabel_=label("—","eyebrow");headerBar->addWidget(uptimeLabel_);dv->insertLayout(2,headerBar);
 auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);
 auto *content=new QWidget;auto *body=new QVBoxLayout(content);body->setContentsMargins(0,0,0,0);body->setSpacing(8);
 auto *strip=new QGridLayout;strip->setSpacing(10);
 QLabel *swapDetail=nullptr,*netDetail=nullptr;
 auto *cpuCard=metricCard(i18n("▣  CPU"),cpuLabel_,cpuDetail_);cpuCard->setObjectName("metric");cpuCard->setProperty("accent","blue");strip->addWidget(cpuCard,0,0);
 auto *memoryCard=metricCard(i18n("▤  Memory"),memLabel_,memDetail_);memoryCard->setObjectName("metric");memoryCard->setProperty("accent","green");strip->addWidget(memoryCard,0,1);
 auto *diskCard=metricCard(i18n("▱  Storage (/)"),diskLabel_,diskDetail_);diskCard->setObjectName("metric");diskCard->setProperty("accent","purple");strip->addWidget(diskCard,0,2);
 auto *swapCard=metricCard(i18n("◉  Swap"),swapLabel_,swapDetail);swapCard->setObjectName("metric");strip->addWidget(swapCard,1,0);
 auto *tempCard=metricCard(i18n("♨  CPU Temp"),temperatureLabel_,temperatureDetail_);tempCard->setObjectName("metric");tempCard->setProperty("accent","orange");strip->addWidget(tempCard,1,1);
 auto *netCard=metricCard(i18n("◉  Network"),netLabel_,netDetail);netCard->setObjectName("metric");netCard->setProperty("accent","blue");strip->addWidget(netCard,1,2);
 for(int i=0;i<3;++i)strip->setColumnStretch(i,1);
 for(int i=0;i<6;++i){auto *w=strip->itemAtPosition(i/3,i%3)->widget();w->setMinimumWidth(0);w->setMinimumHeight(70);w->setMaximumHeight(85);w->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);}
 netLabel_->setStyleSheet("font-size:18px;font-weight:800;");
 body->addLayout(strip);
 auto *graphs=new QGridLayout;graphs->setSpacing(8);
 QVBoxLayout *cpuPanelLayout;auto *cpuPanel=panel(cpuPanelLayout);
 cpuPanelLayout->addWidget(label(i18n("◉  CPU Utilization (Live)"),"eyebrow"));
 cpuChart_=new ChartWidget(QColor("#399dff"));cpuChart_->setMinimumHeight(105);cpuPanelLayout->addWidget(cpuChart_,1);
 QVBoxLayout *memPanelLayout;auto *memPanel=panel(memPanelLayout);
 memPanelLayout->addWidget(label(i18n("◉  Memory Usage (Live)"),"eyebrow"));
 memChart_=new ChartWidget(QColor("#24db91"));memChart_->setMinimumHeight(105);memPanelLayout->addWidget(memChart_,1);
 graphs->addWidget(cpuPanel,0,0);graphs->addWidget(memPanel,0,1);graphs->setColumnStretch(0,1);graphs->setColumnStretch(1,1);
 body->addLayout(graphs);
 auto *lower=new QGridLayout;lower->setSpacing(8);
 QVBoxLayout *thermalPreviewLayout;auto *thermalPreview=panel(thermalPreviewLayout);
 thermalPreviewLayout->addWidget(label(i18n("♨  Thermal & Hardware Health"),"eyebrow"));
 auto *thermalGrid=new QGridLayout;thermalGrid->setSpacing(5);thermalTiles_=thermalGrid;
 for(int i=0;i<8;++i){auto *tile=new QFrame;tile->setObjectName("metric");tile->setProperty("accent",i<4?"green":"blue");auto *v=new QVBoxLayout(tile);v->setContentsMargins(6,4,6,4);v->setSpacing(1);auto *n=label("Detecting…","thermalName");n->setWordWrap(true);auto *t=label("—","thermalValue");v->addWidget(n);v->addWidget(t);thermalNames_.append(n);thermalValues_.append(t);thermalGrid->addWidget(tile,i/4,i%4);}
 thermalPreviewLayout->addLayout(thermalGrid);
 QVBoxLayout *storageLayout;auto *storagePanel=panel(storageLayout);
 storageLayout->addWidget(label(i18n("◈  Storage Overview"),"eyebrow"));
 storageLayout->addWidget(label("▱  / (Linux root partition)","eyebrow"));
 physicalSummary_=label("Detecting complete physical disks…","muted");physicalSummary_->setWordWrap(true);storageLayout->addWidget(physicalSummary_);rootUsageText_=label("Reading disk…","muted");storageLayout->addWidget(rootUsageText_);
 rootUsage_=new QProgressBar;rootUsage_->setRange(0,1000);rootUsage_->setTextVisible(false);storageLayout->addWidget(rootUsage_);
 storageLayout->addSpacing(2);storageLayout->addWidget(label("Physical disk health: SMART access not configured","muted"));storageLayout->addStretch();
 lower->addWidget(thermalPreview,0,0);lower->addWidget(storagePanel,0,1);lower->setColumnStretch(0,1);lower->setColumnStretch(1,1);
 body->addLayout(lower);
 QVBoxLayout *insightLayout;auto *insightPanel=panel(insightLayout);
 insightLayout->addWidget(label(i18n("JATAYU INSIGHTS"),"eyebrow"));insightLabel_=label("—");
 insightLabel_->setWordWrap(true);insightLayout->addWidget(insightLabel_);body->addWidget(insightPanel);

 scroll->setWidget(content);scroll->setToolTip(i18n("Scroll to see Jatayu Insights and additional details"));dv->addWidget(scroll,1);pages_->addWidget(dashboard);
 // Performance: compact live-monitoring workspace with clear metric hierarchy.
 QVBoxLayout *pv;auto *performance=page(pv,i18n("Performance timeline"),i18n("Live CPU and memory trends · 60 recent samples · refresh interval adjustable"));
 pv->setSpacing(10);
 auto *summary=new QHBoxLayout;summary->setSpacing(10);
 auto perfTile=[&](const QString &heading,const QString &accent,QLabel *&number,const QString &caption){
  auto *tile=new QFrame;tile->setObjectName("perfTile");
  tile->setStyleSheet(QString("QFrame#perfTile{background:#142640;border:1px solid %1;border-radius:13px;}").arg(accent));
  auto *v=new QVBoxLayout(tile);v->setContentsMargins(16,11,16,10);v->setSpacing(3);
  auto *name=label(heading,"muted");name->setStyleSheet("font-size:11px;font-weight:800;color:#b7c9e5;");
  number=label("—","value");number->setStyleSheet(QString("font-size:25px;font-weight:800;color:%1;").arg(accent));
  v->addWidget(name);v->addWidget(number);v->addWidget(label(caption,"muted"));
  summary->addWidget(tile,1);
 };
 perfTile(i18n("CPU LOAD"),"#53b4ff",perfSummaryCpu_,i18n("All logical processors"));
 perfTile(i18n("MEMORY IN USE"),"#31d7bc",perfSummaryMem_,i18n("Physical RAM"));
 perfTile(i18n("SWAP IN USE"),"#c3a3ff",perfSwapValue_,i18n("Virtual memory"));
 pv->addLayout(summary);
 auto performancePanel=[&](const QString &title,const QString &subtitle,const QString &accent,QLabel *&value,QLabel *&detail,ChartWidget *&chart){
  QVBoxLayout *layout;auto *frame=panel(layout);
  layout->setContentsMargins(17,12,17,9);layout->setSpacing(5);
  auto *head=new QHBoxLayout;head->setSpacing(10);
  auto *titles=new QVBoxLayout;titles->setSpacing(2);
  auto *heading=label(title,"eyebrow");heading->setStyleSheet("font-size:15px;font-weight:800;color:#f0f6ff;");
  titles->addWidget(heading);titles->addWidget(label(subtitle,"muted"));
  head->addLayout(titles);head->addStretch();
  auto *stats=new QVBoxLayout;stats->setSpacing(0);
  value=label("—","value");value->setAlignment(Qt::AlignRight);
  value->setStyleSheet(QString("font-size:25px;font-weight:800;color:%1;").arg(accent));
  detail=label(" ","muted");detail->setAlignment(Qt::AlignRight);
  stats->addWidget(value);stats->addWidget(detail);head->addLayout(stats);
  layout->addLayout(head);
  chart=new ChartWidget(QColor(accent));chart->setMinimumHeight(100);chart->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  layout->addWidget(chart,1);
  pv->addWidget(frame,1);
 };
 ChartWidget *cpuHistory=nullptr,*memHistory=nullptr;
 performancePanel(i18n("CPU UTILIZATION"),i18n("Processor activity over time"),"#53b4ff",perfCpuValue_,perfCpuDetail_,cpuHistory);
 performancePanel(i18n("MEMORY UTILIZATION"),i18n("RAM usage over time"),"#31d7bc",perfMemValue_,perfMemDetail_,memHistory);
 cpuHistory->setObjectName("cpuHistory");memHistory->setObjectName("memHistory");
 pages_->addWidget(performance);
 // Processes
 QVBoxLayout *procv;auto *processes=page(procv,i18n("Process explorer"),i18n("Read-only table · sorted by resident memory (highest first)"));
 processSearch_=new QLineEdit;processSearch_->setPlaceholderText(i18n("Filter processes by name or PID…"));procv->addWidget(processSearch_);
 processTable_=new QTableWidget(0,3);processTable_->setHorizontalHeaderLabels({i18n("PID"),i18n("Process"),i18n("RAM (MiB)")});setupTable(processTable_);processTable_->setSortingEnabled(false);
 processTable_->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);processTable_->setColumnWidth(0,105);processTable_->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);processTable_->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);processTable_->setColumnWidth(2,135);procv->addWidget(processTable_,1);pages_->addWidget(processes);
 processTable_->setSortingEnabled(true);processTable_->sortItems(2,Qt::DescendingOrder);
 connect(processTable_->horizontalHeader(),&QHeaderView::sortIndicatorChanged,this,[this](int column,Qt::SortOrder order){
 processSortColumn_=column;processSortOrder_=order;});
 connect(processSearch_,&QLineEdit::textChanged,this,[this](const QString &query){for(int i=0;i<processTable_->rowCount();++i){const auto *pid=processTable_->item(i,0);const auto *name=processTable_->item(i,1);processTable_->setRowHidden(i,!(pid&&name&&(pid->text().contains(query,Qt::CaseInsensitive)||name->text().contains(query,Qt::CaseInsensitive))));}});
 // Temperatures
 QVBoxLayout *tv;auto *thermals=page(tv,i18n("Thermal & hardware health"),i18n("Live kernel sensors · device-specific thresholds where available"));
 QVBoxLayout *thp;auto *thermalPanel=panel(thp);thermalSummary_=label("—","eyebrow");thp->addWidget(thermalSummary_);
 thermalChart_=new ChartWidget(QColor("#ffad5c"));thermalChart_->setTemperatureMode(true);thermalChart_->setMinimumHeight(125);thp->addWidget(thermalChart_);tv->addWidget(thermalPanel);
 thermalTable_=new QTableWidget(0,5);thermalTable_->setHorizontalHeaderLabels({i18n("Device"),i18n("Sensor"),i18n("Current °C"),i18n("High °C"),i18n("Critical °C")});setupTable(thermalTable_);
 thermalTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);tv->addWidget(thermalTable_,1);
 tv->addWidget(label(i18n("SMART disk temperatures require a separately authorized integration; not available in this build."),"muted"));pages_->addWidget(thermals);
 // System
 // Navigation order must match stacked page order exactly.
 QVBoxLayout *sdv;auto *storage=page(sdv,i18n("Storage & Disks"),i18n("Physical drives and all detected partitions, including unmounted Windows volumes"));
 QVBoxLayout *physicalLayout;auto *physicalPanel=panel(physicalLayout);
 physicalLayout->addWidget(label(i18n("Physical drives and partitions (read-only)"),"eyebrow"));
 blockTable_=new QTableWidget(0,6);blockTable_->setHorizontalHeaderLabels({i18n("Device"),i18n("Path"),i18n("Type"),i18n("Capacity"),i18n("Model / filesystem"),i18n("Mount")});setupTable(blockTable_);blockTable_->setMinimumHeight(230);blockTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);blockTable_->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);physicalLayout->addWidget(blockTable_);sdv->addWidget(physicalPanel,2);
 QVBoxLayout *stPanelLayout;auto *stPanel=panel(stPanelLayout);
 stPanelLayout->addWidget(label(i18n("Mounted filesystem usage"),"eyebrow"));
 storageTable_=new QTableWidget(0,5);storageTable_->setHorizontalHeaderLabels({i18n("Mount point"),i18n("Filesystem"),i18n("Total GiB"),i18n("Used GiB"),i18n("Used %")});setupTable(storageTable_);storageTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
 stPanelLayout->addWidget(storageTable_);sdv->addWidget(stPanel,1);
 sdv->addWidget(label(i18n("SMART disk health requires separately authorized access; no elevated privileges are requested."),"muted"));pages_->addWidget(storage);
 QVBoxLayout *nv;auto *network=page(nv,i18n("Network"),i18n("Live download/upload rates and interface statistics"));
 auto *trafficRow=new QHBoxLayout;trafficRow->setSpacing(10);
 auto trafficPanel=[&](const QString &title,const QString &color,QLabel *&value,ChartWidget *&chart){
 QVBoxLayout *v;auto *frame=panel(v);v->setContentsMargins(14,12,14,10);
 auto *head=new QHBoxLayout;head->addWidget(label(title,"eyebrow"));head->addStretch();
 value=label("—","value");value->setStyleSheet(QString("font-size:20px;font-weight:800;color:%1;").arg(color));head->addWidget(value);v->addLayout(head);
 chart=new ChartWidget(QColor(color));chart->setTrafficMode(true);chart->setMinimumHeight(100);v->addWidget(chart,1);
 trafficRow->addWidget(frame,1);
 };
 trafficPanel(i18n("↓  DOWNLOAD"),"#53b4ff",networkRxValue_,networkRxChart_);
 trafficPanel(i18n("↑  UPLOAD"),"#31d7bc",networkTxValue_,networkTxChart_);
 nv->addLayout(trafficRow,1);
 nv->addWidget(label(i18n("Graphs automatically scale to recent traffic; speeds use KiB/s or MiB/s."),"muted"));
 QVBoxLayout *np;auto *networkPanel=panel(np);np->addWidget(label(i18n("Detected network interfaces"),"eyebrow"));
 networkTable_=new QTableWidget(0,6);
 networkTable_->setHorizontalHeaderLabels({i18n("Interface"),i18n("Status"),i18n("IPv4 address"),i18n("Hardware address"),i18n("Received"),i18n("Sent")});
 setupTable(networkTable_);networkTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
 np->addWidget(networkTable_);nv->addWidget(networkPanel,2);pages_->addWidget(network);
 QVBoxLayout *iv;auto *info=page(iv,i18n("System Info"),i18n("Detailed hardware, operating system and runtime information"));
 auto *infoScroll=new QScrollArea;infoScroll->setWidgetResizable(true);auto *infoBody=new QWidget;auto *infoV=new QVBoxLayout(infoBody);infoV->setSpacing(10);
 auto section=[&](const QString &heading,const QVector<QPair<QString,QString>> &rows){
 QVBoxLayout *layout;auto *frame=panel(layout);layout->setContentsMargins(18,16,18,18);layout->setSpacing(12);
 auto *headingLabel=label(heading,"eyebrow");headingLabel->setStyleSheet("font-size:15px;font-weight:800;color:#6ad6ff;");layout->addWidget(headingLabel);
 auto *grid=new QGridLayout;grid->setHorizontalSpacing(22);grid->setVerticalSpacing(12);int half=(rows.size()+1)/2;
 for(int i=0;i<rows.size();++i){int col=i/half;int row=i%half;auto *cell=new QWidget;auto *v=new QVBoxLayout(cell);v->setContentsMargins(0,0,4,0);v->setSpacing(3);
 auto *key=label(rows[i].first.toUpper());key->setStyleSheet("font-size:11px;font-weight:700;color:#8eaacc;");
 auto *val=label(rows[i].second);val->setWordWrap(true);val->setTextInteractionFlags(Qt::TextSelectableByMouse);val->setStyleSheet("font-size:14px;font-weight:600;color:#f1f7ff;");v->addWidget(key);v->addWidget(val);grid->addWidget(cell,row,col);}
 grid->setColumnStretch(0,1);grid->setColumnStretch(1,1);layout->addLayout(grid);infoV->addWidget(frame);};
 const auto memSnapshot=Metrics::readMemory();
 section(i18n("OPERATING SYSTEM"),{{"Distribution",QSysInfo::prettyProductName()},{"Kernel",QSysInfo::kernelVersion()},{"Architecture",QSysInfo::currentCpuArchitecture()},{"Hostname",QSysInfo::machineHostName()},{"Qt runtime",QString::fromLatin1(qVersion())},{"Desktop session",qEnvironmentVariable("XDG_CURRENT_DESKTOP","Unknown")},{"Display server",qEnvironmentVariable("XDG_SESSION_TYPE","Unknown")}});
 section(i18n("PROCESSOR & MEMORY"),{{"CPU model",cpuModel()},{"Logical processors",QString::number(QThread::idealThreadCount())},{"Installed RAM (reported)",QString::number(memSnapshot.totalKiB/1048576.0,'f',2)+" GiB"},{"Swap",QString::number(memSnapshot.swapTotalKiB/1048576.0,'f',2)+" GiB"}});
 section(i18n("HARDWARE / FIRMWARE"),{{"System manufacturer",sysDmi("sys_vendor")},{"System model",sysDmi("product_name")},{"Board manufacturer",sysDmi("board_vendor")},{"Motherboard",sysDmi("board_name")},{"BIOS vendor",sysDmi("bios_vendor")},{"BIOS version",sysDmi("bios_version")}});
 infoV->addStretch();infoScroll->setWidget(infoBody);iv->addWidget(infoScroll,1);pages_->addWidget(info);
 QVBoxLayout *settingsLayout;auto *settings=page(settingsLayout,i18n("Settings"),i18n("Customize monitoring behavior"));
 QVBoxLayout *settingsPanelLayout;auto *settingsPanel=panel(settingsPanelLayout);
 settingsPanelLayout->addWidget(label(i18n("Refresh interval"),"eyebrow"));
 auto *interval=new QComboBox;interval->addItem(i18n("1 second"),1000);interval->addItem(i18n("1.5 seconds (default)"),1500);interval->addItem(i18n("3 seconds"),3000);interval->addItem(i18n("5 seconds"),5000);QSettings saved(QStringLiteral("JatayuOS"),QStringLiteral("JatayuPulse"));
 const int savedInterval=saved.value(QStringLiteral("refreshIntervalMs"),1500).toInt();
 int savedIndex=interval->findData(savedInterval);interval->setCurrentIndex(savedIndex>=0?savedIndex:1);
 settingsPanelLayout->addWidget(interval);settingsPanelLayout->addWidget(label(i18n("Changes take effect immediately. No privileged operations are performed."),"muted"));
 connect(interval,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this,interval](int index){const int ms=interval->itemData(index).toInt();
 if(refreshTimer_)refreshTimer_->setInterval(ms);
 QSettings(QStringLiteral("JatayuOS"),QStringLiteral("JatayuPulse")).setValue(QStringLiteral("refreshIntervalMs"),ms);});
 settingsLayout->addWidget(settingsPanel);settingsLayout->addStretch();pages_->addWidget(settings);
 QVBoxLayout *aboutLayout;auto *about=page(aboutLayout,i18n("About Jatayu Pulse"),i18n("A native system monitoring application built for JatayuOS"));
 QVBoxLayout *aboutPanelLayout;auto *aboutPanel=panel(aboutPanelLayout);
 auto *hero=new QHBoxLayout;auto *heroIcon=new QLabel;heroIcon->setPixmap(windowIcon().pixmap(QSize(72,72)));hero->addWidget(heroIcon);auto *heroText=new QVBoxLayout;heroText->addWidget(label("Jatayu Pulse","value"));heroText->addWidget(label("Native monitoring for JatayuOS","muted"));hero->addLayout(heroText);hero->addStretch();aboutPanelLayout->addLayout(hero);aboutPanelLayout->addWidget(detailLine("Creator","Anup Kumar Yadav"));
 aboutPanelLayout->addWidget(detailLine("Version","1.0.1 — Security maintenance"));
 aboutPanelLayout->addWidget(detailLine("Platform","JatayuOS Udaan · Debian 13 · KDE Plasma 6"));
 aboutPanelLayout->addWidget(detailLine("Technology","C++17 · Qt 6 Widgets · KDE Frameworks 6"));
 aboutPanelLayout->addWidget(detailLine("License","GNU General Public License v3.0 or later (GPL-3.0-or-later)"));
 aboutPanelLayout->addWidget(detailLine("Purpose","Read-only CPU, memory, process, thermal, network and storage monitoring"));
 auto *site=new QPushButton(i18n("Visit JatayuOS website — jatayuos.in"));site->setCursor(Qt::PointingHandCursor);site->setStyleSheet("QPushButton{background:#086cf0;color:white;padding:12px;border-radius:8px;text-align:left;font-weight:700;}QPushButton:hover{background:#2587ff;}");
 connect(site,&QPushButton::clicked,this,[]{QDesktopServices::openUrl(QUrl(QStringLiteral("https://jatayuos.in")));});aboutPanelLayout->addWidget(site);
 auto *license=new QPushButton(i18n("Read the GNU GPL v3 license"));license->setCursor(Qt::PointingHandCursor);license->setStyleSheet("QPushButton{background:#263956;color:#eef5ff;padding:10px;border-radius:8px;text-align:left;}QPushButton:hover{background:#315d91;}");
 connect(license,&QPushButton::clicked,this,[]{QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.gnu.org/licenses/gpl-3.0.html")));});aboutPanelLayout->addWidget(license);
 auto *limitations=label(i18n("Privacy and permissions: System information is read locally. Jatayu Pulse does not require administrator privileges for ordinary monitoring. Hardware readings depend on kernel drivers, permissions and installed utilities."),"muted");limitations->setWordWrap(true);aboutPanelLayout->addWidget(limitations);
 aboutLayout->addWidget(aboutPanel);aboutLayout->addStretch();pages_->addWidget(about);

 selectPage(0);
 refreshTimer_=new QTimer(this);connect(refreshTimer_,&QTimer::timeout,this,&MainWindow::refresh);refreshTimer_->start(interval->currentData().toInt());refresh();
}
void MainWindow::selectPage(int index){pages_->setCurrentIndex(index);for(int i=0;i<navButtons_.size();++i){navButtons_[i]->setProperty("active",i==index);navButtons_[i]->style()->unpolish(navButtons_[i]);navButtons_[i]->style()->polish(navButtons_[i]);}}
void MainWindow::refresh(){
 const auto cpu=Metrics::readCpu();const auto mem=Metrics::readMemory();const auto net=Metrics::readNetwork();
 const double elapsedSeconds=networkClock_.isValid()?double(networkClock_.restart())/1000.0:0.0;
 if(!networkClock_.isValid())networkClock_.start();
 const double cpuUse=initialized_?Metrics::cpuPercent(lastCpu_,cpu):0;
 const double memUse=MonitorHealth::usagePercent(mem.totalKiB,mem.availableKiB);
 const double swapUse=MonitorHealth::usagePercent(mem.swapTotalKiB,mem.swapFreeKiB);
 if(perfCpuValue_)perfCpuValue_->setText(initialized_?QString::number(cpuUse,'f',1)+"%":i18n("Measuring…"));
 if(perfMemValue_)perfMemValue_->setText(QString::number(memUse,'f',1)+"%");
 if(perfSwapValue_)perfSwapValue_->setText(QString::number(swapUse,'f',1)+"%");
 if(perfCpuDetail_)perfCpuDetail_->setText(i18n("Current usage"));
 if(perfMemDetail_)perfMemDetail_->setText(QString("%1 / %2 GiB").arg((mem.totalKiB-std::min(mem.availableKiB,mem.totalKiB))/1048576.0,0,'f',1).arg(mem.totalKiB/1048576.0,0,'f',1));
 if(perfSummaryCpu_)perfSummaryCpu_->setText(initialized_?QString::number(cpuUse,'f',1)+"%":i18n("Measuring…"));
 if(perfSummaryMem_)perfSummaryMem_->setText(QString::number(memUse,'f',1)+"%");
 cpuLabel_->setText(initialized_?QString::number(cpuUse,'f',1)+"%":i18n("Measuring…"));
 memLabel_->setText(QString::number(memUse,'f',1)+"%");
 cpuDetail_->setText(i18n("Live CPU utilization"));memDetail_->setText(QString("%1 / %2 GiB").arg((mem.totalKiB-std::min(mem.availableKiB,mem.totalKiB))/1048576.0,0,'f',1).arg(mem.totalKiB/1048576.0,0,'f',1));
 swapLabel_->setText(QString::number(swapUse,'f',1)+"%");
 if(initialized_){const double rxKiB=NetworkRate::kibPerSecond(lastNetwork_.rx,net.rx,elapsedSeconds);
 const double txKiB=NetworkRate::kibPerSecond(lastNetwork_.tx,net.tx,elapsedSeconds);
 netLabel_->setText(QString("↓ %1 KiB/s\n↑ %2 KiB/s").arg(rxKiB,0,'f',1).arg(txKiB,0,'f',1));
 if(networkRxValue_)networkRxValue_->setText(QString::number(rxKiB,'f',1)+" KiB/s");
 if(networkTxValue_)networkTxValue_->setText(QString::number(txKiB,'f',1)+" KiB/s");
 if(networkRxChart_)networkRxChart_->appendValue(rxKiB);
 if(networkTxChart_)networkTxChart_->appendValue(txKiB);
 cpuChart_->appendValue(cpuUse);memChart_->appendValue(memUse);
 if(auto *c=findChild<ChartWidget*>("cpuHistory"))c->appendValue(cpuUse);
 if(auto *c=findChild<ChartWidget*>("memHistory"))c->appendValue(memUse);
 }else netLabel_->setText(i18n("Measuring…"));
 uptimeLabel_->setText(i18n("System uptime: %1",Metrics::uptime()));
 insightLabel_->setText(memUse>90?i18n("Memory pressure is high. Inspect memory-heavy processes."):cpuUse>90?i18n("CPU usage is high. Inspect running processes."):i18n("Resource usage is within the basic alert thresholds."));
 const auto list=Metrics::processes(150);
 QSignalBlocker sortingSignals(processTable_->horizontalHeader());
 processTable_->setSortingEnabled(false);processTable_->setRowCount(list.size());
 const auto query=processSearch_->text();
 for(int i=0;i<list.size();++i){auto *pidItem=new QTableWidgetItem;pidItem->setData(Qt::DisplayRole,list[i].pid);processTable_->setItem(i,0,pidItem);auto *nameItem=new QTableWidgetItem(list[i].name);nameItem->setToolTip(list[i].name);processTable_->setItem(i,1,nameItem);auto *ramItem=new QTableWidgetItem;ramItem->setData(Qt::DisplayRole,list[i].rssKiB/1024.0);ramItem->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);processTable_->setItem(i,2,ramItem);
 processTable_->setRowHidden(i,!(list[i].name.contains(query,Qt::CaseInsensitive)||QString::number(list[i].pid).contains(query,Qt::CaseInsensitive)));}
 processTable_->setSortingEnabled(true);processTable_->sortItems(processSortColumn_,processSortOrder_);
 const QStorageInfo rootDisk(QStringLiteral("/"));
 if(rootDisk.isValid() && rootDisk.isReady() && rootDisk.bytesTotal()>0){
  const double used=100.0*double(rootDisk.bytesTotal()-rootDisk.bytesAvailable())/double(rootDisk.bytesTotal());
  diskLabel_->setText(QString::number(used,'f',1)+"%");
  diskDetail_->setText(QString("%1 / %2 GiB used").arg((rootDisk.bytesTotal()-rootDisk.bytesAvailable())/1073741824.0,0,'f',1).arg(rootDisk.bytesTotal()/1073741824.0,0,'f',1));
  rootUsage_->setValue(int(used*10));rootUsageText_->setText(QString("%1 / %2 GiB used  ·  %3%").arg((rootDisk.bytesTotal()-rootDisk.bytesAvailable())/1073741824.0,0,'f',1).arg(rootDisk.bytesTotal()/1073741824.0,0,'f',1).arg(used,0,'f',1));
 }else{diskLabel_->setText(i18n("Unavailable"));diskDetail_->clear();}
 // Populate the dedicated storage and network pages using unprivileged Qt APIs.
 if(blockTable_ && (++blockRefreshTick_%5==1)){
 refreshBlockDevices(blockTable_);
 if(physicalSummary_){double totalGiB=0;int drives=0;for(int i=0;i<blockTable_->rowCount();++i){if(blockTable_->item(i,2)&&blockTable_->item(i,2)->text()=="disk"){++drives;const QString v=blockTable_->item(i,3)->text();totalGiB+=v.section(' ',0,0).toDouble();}}
 physicalSummary_->setText(drives?QString("Physical disks: %1 · combined capacity: %2 GiB (includes unmounted partitions)").arg(drives).arg(totalGiB,0,'f',1):"Physical disk details unavailable; open Storage & Disks");}}

 if(storageTable_){const auto volumes=QStorageInfo::mountedVolumes();storageTable_->setRowCount(0);
 for(const auto &volume:volumes){if(!volume.isValid()||!volume.isReady()||volume.bytesTotal()<=0)continue;
 const int row=storageTable_->rowCount();storageTable_->insertRow(row);
 const double total=volume.bytesTotal()/1073741824.0,used=(volume.bytesTotal()-volume.bytesAvailable())/1073741824.0;
 const QString cells[]={volume.rootPath(),QString::fromUtf8(volume.fileSystemType()),QString::number(total,'f',1),QString::number(used,'f',1),QString::number(100.0*used/total,'f',1)+"%"};
 for(int col=0;col<5;++col)storageTable_->setItem(row,col,new QTableWidgetItem(cells[col]));}}
 if(networkTable_){const auto interfaces=QNetworkInterface::allInterfaces();networkTable_->setRowCount(interfaces.size());
 for(int row=0;row<interfaces.size();++row){const auto &iface=interfaces[row];QString ipv4="—";
 for(const auto &entry:iface.addressEntries())if(entry.ip().protocol()==QAbstractSocket::IPv4Protocol){ipv4=entry.ip().toString();break;}
 const QString interfaceName=iface.name();
 const QString rxBytes=interfaceCounter(interfaceName,"rx_bytes");
 const QString txBytes=interfaceCounter(interfaceName,"tx_bytes");
 const QString cells[]={iface.humanReadableName(),iface.flags().testFlag(QNetworkInterface::IsUp)?i18n("Up"):i18n("Down"),ipv4,iface.hardwareAddress(),rxBytes,txBytes};
 for(int col=0;col<6;++col)networkTable_->setItem(row,col,new QTableWidgetItem(cells[col]));}}
 const auto readings=Thermal::discover();thermalSummary_->setText(i18np("%1 kernel temperature sensor detected","%1 kernel temperature sensors detected",readings.size()));
 thermalTable_->setRowCount(readings.size());double maxTemp=-1000;double cpuTemp=-1000;
 for(int i=0;i<readings.size();++i){const auto &r=readings[i];maxTemp=std::max(maxTemp,r.celsius);
 if(r.device.startsWith("coretemp",Qt::CaseInsensitive)||r.device.startsWith("k10temp",Qt::CaseInsensitive))cpuTemp=std::max(cpuTemp,r.celsius);
 const QString cells[]={friendlyDevice(r.device),r.label,QString::number(r.celsius,'f',1),r.hasHigh?QString::number(r.highCelsius,'f',1):QStringLiteral("—"),r.hasCritical?QString::number(r.criticalCelsius,'f',1):QStringLiteral("—")};
 for(int j=0;j<5;++j)thermalTable_->setItem(i,j,new QTableWidgetItem(cells[j]));}
 QVector<ThermalReading> ordered=readings;
 std::stable_sort(ordered.begin(),ordered.end(),[](const ThermalReading &a,const ThermalReading &b){return (a.device.startsWith("coretemp")||a.device.startsWith("k10temp")) > (b.device.startsWith("coretemp")||b.device.startsWith("k10temp"));});
 for(int i=0;i<thermalNames_.size();++i){if(i<ordered.size()){const auto &r=ordered[i];QString name=friendlyDevice(r.device);if(r.device.startsWith("coretemp")||r.device.startsWith("k10temp"))name="CPU "+r.label;else if(r.device.startsWith("iwlwifi"))name="Intel Wi-Fi";else if(r.device.startsWith("acpitz"))name="ACPI Thermal";thermalNames_[i]->setText(name);thermalValues_[i]->setText(QString::number(r.celsius,'f',1)+"°C");}else{thermalNames_[i]->setText("Sensor unavailable");thermalValues_[i]->setText("—");}}
 if(maxTemp>-999){thermalChart_->appendValue(std::clamp(maxTemp,0.0,100.0));
  temperatureLabel_->setText(cpuTemp>-999?QString::number(cpuTemp,'f',1)+"°C":i18n("N/A"));
  temperatureDetail_->setText(cpuTemp>-999?i18n("Hottest detected CPU sensor"):i18n("CPU temperature sensor unavailable"));
 }else{temperatureLabel_->setText(i18n("N/A"));temperatureDetail_->setText(i18n("No accessible sensors"));}
 healthLabel_->setText(cpuUse>90||memUse>90?i18n("●  High resource usage"):i18n("●  System monitoring"));
 lastCpu_=cpu;lastNetwork_=net;initialized_=true;
}
