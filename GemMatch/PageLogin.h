#ifndef PAGELOGIN_H
#define PAGELOGIN_H

#include <QWidget>
#include <QLineEdit>

class MainWindow;

class PageLogin : public QWidget {
    Q_OBJECT
public:
    explicit PageLogin(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    QLineEdit* m_editUser;
    QLineEdit* m_editPass;
    QLineEdit* m_editEmail;

    void setupUI();
    void onLoginClicked();
    void onRegisterClicked();
protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // PAGELOGIN_H