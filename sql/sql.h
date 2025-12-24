#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_sql.h"

class sql : public QMainWindow
{
    Q_OBJECT

public:
    sql(QWidget *parent = nullptr);
    ~sql();

private:
    Ui::sqlClass ui;
};

