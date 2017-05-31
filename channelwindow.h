#ifndef CHANNELWINDOW_H
#define CHANNELWINDOW_H

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QTextEdit;

class ChannelWindow : public QWidget
{
    Q_OBJECT
private:
    QHBoxLayout* pHLayout;
    QVBoxLayout* pVLayout;
    QPushButton* pAddButton;
    QTextEdit* pTextArea;



public:
    explicit ChannelWindow(QWidget *parent = 0);

signals:

public slots:
};

#endif // CHANNELWINDOW_H
