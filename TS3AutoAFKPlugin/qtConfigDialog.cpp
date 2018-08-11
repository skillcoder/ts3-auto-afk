#pragma once
#include <QtCore/QSettings>
#include "qtConfigDialog.h"
#include "globals.h"

qtConfigDialog::qtConfigDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
	ui.spinBox_secondsForIdle->setValue(cfg.value("secondsForIdle", DEFAULTSECONDSFORIDLE).toUInt());
	ui.checkBox_manipulateMic->setChecked(cfg.value("manipulateMic", DEFAULTMANIPULATEMIC).toBool());
	ui.checkBox_manipulateSound->setChecked(cfg.value("manipulateSound", DEFAULTMANIPULATESOUND).toBool());
	ui.checkBox_disableWhenFullscreen->setChecked(cfg.value("disableWhenFullscreen", DEFAULTDISABLEWHENFULLSCREEN).toBool());
}

qtConfigDialog::~qtConfigDialog()
{
}

void qtConfigDialog::accept() {
	Globals::secondsForIdle = ui.spinBox_secondsForIdle->value();
	Globals::manipulateMic = ui.checkBox_manipulateMic->isChecked();
	Globals::manipulateSound = ui.checkBox_manipulateSound->isChecked();
	Globals::disableWhenFullscreen = ui.checkBox_disableWhenFullscreen->isChecked();

	QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
	cfg.setValue("secondsForIdle", Globals::secondsForIdle);
	cfg.setValue("manipulateMic", Globals::manipulateMic);
	cfg.setValue("manipulateSound", Globals::manipulateSound);
	cfg.setValue("disableWhenFullscreen", Globals::disableWhenFullscreen);
	QDialog::accept();
}

void qtConfigDialog::reject() {
	QDialog::reject();
}
