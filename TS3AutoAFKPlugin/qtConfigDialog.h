#pragma once

#include <QDialog>
#include "ui_qtConfigDialog.h"

class qtConfigDialog : public QDialog
{
	Q_OBJECT

public:
	qtConfigDialog(QWidget *parent = Q_NULLPTR);
	~qtConfigDialog();

private:
	Ui::qtConfigDialog ui;
	void qtConfigDialog::accept();
	void qtConfigDialog::reject();
};
