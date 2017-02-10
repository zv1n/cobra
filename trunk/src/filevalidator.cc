/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "filevalidator.h"

#include <QFile>

cobraFileValidator::cobraFileValidator()
{
}

QValidator::State
cobraFileValidator::validate(QString& input, int& pos) const
{
    QFile file;
    (void)pos;

    if (file.exists(input))
        return QValidator::Acceptable;

    return QValidator::Intermediate;
}
