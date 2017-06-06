#include "channelsettingsdialog.h"
#include "ui_channelsettingsdialog.h"
#include "channel.h"

ChannelSettingsDialog::ChannelSettingsDialog(Channel* channel, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChannelSettingsDialog)
    , pChannel(channel)
{
    ui->setupUi(this);

    ui->nameEdit->setText(pChannel->getChannelName());
    ui->welcomeEdit->setText(pChannel->getWelcome());
    ui->numOfGuests->setValue(pChannel->getMaxGuestsNum());
    ui->descriptEdit->setText(pChannel->getDescription());
    ui->checkBox->setChecked(pChannel->getPrivateStatus());
    if (ui->checkBox->isChecked()) {
        ui->passwordEdit->setReadOnly(false);
        ui->passwordEdit->setText(pChannel->getPassword());
    }
    else {
        ui->passwordEdit->setReadOnly(true);
        ui->passwordEdit->setText("");
    }

    connect(ui->checkBox, SIGNAL(clicked()),
                this, SLOT(slotStateChanged()));

    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(slotAccepted()));
}

ChannelSettingsDialog::~ChannelSettingsDialog()
{
    delete ui;
}

void ChannelSettingsDialog::slotStateChanged() {
    if (ui->checkBox->isChecked())
        ui->passwordEdit->setReadOnly(false);
    else
        ui->passwordEdit->setReadOnly(true);
}

void ChannelSettingsDialog::slotAccepted() {
    pChannel->setChannelName(ui->nameEdit->text());
    pChannel->setChannelWelcome(ui->welcomeEdit->text());
    pChannel->setMaxGuestsNum(ui->numOfGuests->value());
    pChannel->setChannelDesciption(ui->descriptEdit->toPlainText());
    pChannel->setPrivate(ui->checkBox->isChecked());
    if (pChannel->getPrivateStatus())
        pChannel->setPassword(ui->passwordEdit->text());
    else
        pChannel->setPassword("");
    emit settingsSeted();
}
