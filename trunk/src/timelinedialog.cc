/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "include/timelinedialog.h"
#include "ui_timelinedialog.h"
#include <QMessageBox>

cobraTimelineDialog::cobraTimelineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::timelineDialog)
{
    ui->setupUi(this);
    ui->saveBtn->setDefault(true);
}

cobraTimelineDialog::~cobraTimelineDialog()
{
    delete ui;
}

void
cobraTimelineDialog::setTitle(QString title)
{
    ui->title->setText(title);
}

void
cobraTimelineDialog::setDescription(QString description)
{
    ui->comment->setText(description);
}

QString
cobraTimelineDialog::getTitle() const
{
    return ui->title->text();
}

QString
cobraTimelineDialog::getDescription() const
{
    return ui->comment->text();
}

void
cobraTimelineDialog::on_cnclBtn_clicked()
{
    reject();
}

void cobraTimelineDialog::on_saveBtn_clicked()
{
    accept();
}
