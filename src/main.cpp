#include "mainwindow.h"
#include <QApplication>
#include <KLocalizedString>
#include <KAboutData>
int main(int argc,char **argv) {
 QApplication app(argc,argv);
 KLocalizedString::setApplicationDomain("jatayu-system-monitor");
 KAboutData about(QStringLiteral("jatayu-system-monitor"),i18n("Jatayu System Monitor"),QStringLiteral("0.3.0"),i18n("An original system monitoring dashboard for JatayuOS"),KAboutLicense::GPL_V3);
 KAboutData::setApplicationData(about);
 MainWindow window;window.show();return app.exec();
}
