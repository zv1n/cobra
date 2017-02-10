/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "include/markerdialog.h"
#include "ui_markerdialog.h"
#include <QMessageBox>

markerDialog::markerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::markerDialog)
{
    ui->setupUi(this);
    ui->saveBtn->setDefault(true);
}

markerDialog::~markerDialog()
{
    delete ui;
}

void
markerDialog::setComment(QString comment)
{
    ui->comment->setText(comment);
}

QString
markerDialog::getComment() const
{
    return ui->comment->text();
}

void markerDialog::on_cnclBtn_clicked()
{
    reject();
}

void markerDialog::on_saveBtn_clicked()
{
    accept();
}
