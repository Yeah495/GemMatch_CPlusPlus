#pragma once
#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>

class GameOverDialog : public QDialog {
    Q_OBJECT

public:
    explicit GameOverDialog(int score, QWidget* parent = nullptr);
    ~GameOverDialog();

private:
    void setupUI();
    QPixmap generateQRCode(const QString& text, int size = 200);
    QString generateShareUrl(int score);

    int m_score;
    QLabel* m_titleLabel;
    QLabel* m_scoreLabel;
    QLabel* m_qrLabel;
    QLabel* m_hintLabel;
    QPushButton* m_restartBtn;
    QPushButton* m_menuBtn;

signals:
    void restartGame();
    void backToMenu();
};

#endif // GAMEOVERDIALOG_H
