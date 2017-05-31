#include <QStackedWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "choosechannelwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pStackOfWidgets = new QStackedWidget();

    pChannelsWidget = new ChooseChannelWidget(this);

    this->setCentralWidget(pStackOfWidgets);
}

MainWindow::~MainWindow()
{
    delete ui;
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
