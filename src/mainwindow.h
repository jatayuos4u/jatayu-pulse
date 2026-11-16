#pragma once
#include <QMainWindow>
#include "metrics.h"
#include <QElapsedTimer>
class QLabel;
class QTableWidget;
class ChartWidget;
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QProgressBar;
class QTimer;
class MainWindow : public QMainWindow {
 Q_OBJECT
public:
 explicit MainWindow(QWidget *parent=nullptr);
private:
 void refresh();
 void selectPage(int index);
 QLabel *cpuLabel_=nullptr,*memLabel_=nullptr,*swapLabel_=nullptr,*netLabel_=nullptr,*uptimeLabel_=nullptr,*insightLabel_=nullptr;
 QGridLayout *thermalTiles_=nullptr;
 QProgressBar *rootUsage_=nullptr;
 QTableWidget *storageTable_=nullptr,*networkTable_=nullptr,*blockTable_=nullptr;
 int blockRefreshTick_=0;
 QTimer *refreshTimer_=nullptr;
 ChartWidget *networkRxChart_=nullptr,*networkTxChart_=nullptr;
 QLabel *networkRxValue_=nullptr,*networkTxValue_=nullptr;
 QElapsedTimer networkClock_;
 int processSortColumn_=2;
 Qt::SortOrder processSortOrder_=Qt::DescendingOrder;
 QLabel *rootUsageText_=nullptr,*physicalSummary_=nullptr;
 QVector<QLabel*> thermalNames_,thermalValues_;
 QLabel *perfSummaryCpu_=nullptr,*perfSummaryMem_=nullptr,*perfCpuValue_=nullptr,*perfMemValue_=nullptr,*perfCpuDetail_=nullptr,*perfMemDetail_=nullptr,*perfSwapValue_=nullptr;
 QLabel *healthLabel_=nullptr;
 QLabel *cpuDetail_=nullptr,*memDetail_=nullptr,*thermalSummary_=nullptr,*diskLabel_=nullptr,*diskDetail_=nullptr,*temperatureLabel_=nullptr,*temperatureDetail_=nullptr,*headerTitle_=nullptr;
 ChartWidget *cpuChart_=nullptr,*memChart_=nullptr,*thermalChart_=nullptr;
 QTableWidget *processTable_=nullptr,*thermalTable_=nullptr;
 QStackedWidget *pages_=nullptr;
 QLineEdit *processSearch_=nullptr;
 QVector<QPushButton*> navButtons_;
 CpuTimes lastCpu_;
 NetworkTotals lastNetwork_;
 bool initialized_=false;
};
