/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef TIMELINEDIALOG_H
#define TIMELINEDIALOG_H

#include <QDialog>
#include "timelinetree.h"

namespace Ui {
class timelineDialog;
}

class cobraTimelineDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit cobraTimelineDialog(QWidget *parent = 0);
    ~cobraTimelineDialog();

    void setTitle(QString title);
    void setDescription(QString desc);
    QString getTitle() const;
    QString getDescription() const;

private slots:
    void on_cnclBtn_clicked();
    void on_saveBtn_clicked();

private:
    Ui::timelineDialog *ui;
};

#endif // TIMELINEDIALOG_H
