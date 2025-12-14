#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GemMatch.h"

class GemMatch : public QMainWindow
{
    Q_OBJECT

public:
    GemMatch(QWidget *parent = nullptr);
    ~GemMatch();

    int a;
    int b;
	int c;
private:
    Ui::GemMatchClass ui;
};

