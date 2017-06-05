#include <QStackedWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "choosechannelwidget.h"
#include "usernamesettingwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pStackOfWidgets = new QStackedWidget();
    pChannelsWidget = nullptr;
    pUserNameSetting = new UserNameSettingWidget(this);
    pStackOfWidgets->addWidget(pUserNameSetting);
    connect(pUserNameSetting, SIGNAL(usernameAccepted(QString)),
            this, SLOT(slotUserNameAccepted(QString)));
    pStackOfWidgets->setCurrentWidget(pUserNameSetting);

    this->setCentralWidget(pStackOfWidgets);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (pChannelsWidget != nullptr)
        delete pChannelsWidget;
    if (pUserNameSetting != nullptr)
        delete pUserNameSetting;
}

int MainWindow::addWidget(QWidget* widget) {
    return pStackOfWidgets->addWidget(widget);
}

void MainWindow::setWidget(QWidget* widget) {
    pStackOfWidgets->setCurrentWidget(widget);
}

void MainWindow::removeWidget(QWidget* widget) {
    pStackOfWidgets->removeWidget(widget);
}

QString MainWindow::getUserName() {
    return userName;
}

void MainWindow::slotUserNameAccepted(const QString &username) {
    userName = username;
    pStackOfWidgets->removeWidget(pUserNameSetting);
    delete pUserNameSetting;
    pUserNameSetting = nullptr;
    pChannelsWidget = new ChooseChannelWidget(this);
}
