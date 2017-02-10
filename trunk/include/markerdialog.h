/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef MARKERDIALOG_H
#define MARKERDIALOG_H

#include <QDialog>
#include "timelinetree.h"

namespace Ui {
class markerDialog;
}

class markerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit markerDialog(QWidget *parent = 0);
    ~markerDialog();

    void setComment(QString);
    QString getComment() const;

private slots:
    void on_cnclBtn_clicked();
    void on_saveBtn_clicked();

private:
    Ui::markerDialog *ui;
};

#endif // MARKERDIALOG_H
