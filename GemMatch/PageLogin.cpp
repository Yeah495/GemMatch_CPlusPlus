#include "PageLogin.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

PageLogin::PageLogin(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageLogin::setupUI() {
    // 1. 给当前窗口起一个唯一的 ID 名字
    this->setObjectName("LoginBackground");

    // 2.
    this->setStyleSheet("#LoginBackground { border-image: url(assets/images/bg_login.jpg); }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // 半透明容器
    QWidget* container = new QWidget(this);
    container->setFixedSize(320, 450);
    container->setStyleSheet("background-color: rgba(0, 0, 0, 180); border-radius: 15px;");

    QVBoxLayout* formLayout = new QVBoxLayout(container);
    formLayout->setContentsMargins(30, 40, 30, 40);
    formLayout->setSpacing(15);

    // 标题
    QLabel* title = new QLabel("GEM MATCH");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: gold; font-size: 28px; font-weight: bold; margin-bottom: 20px;");

    // 输入框样式
    QString editStyle = "QLineEdit { padding: 8px; border-radius: 5px; border: 1px solid #555; background: #333; color: white; } "
        "QLineEdit:focus { border: 1px solid gold; }";

    m_editUser = new QLineEdit();
    m_editUser->setPlaceholderText("用户名");
    m_editUser->setStyleSheet(editStyle);

    m_editPass = new QLineEdit();
    m_editPass->setPlaceholderText("密码");
    m_editPass->setEchoMode(QLineEdit::Password);
    m_editPass->setStyleSheet(editStyle);

    m_editEmail = new QLineEdit();
    m_editEmail->setPlaceholderText("邮箱");
    m_editEmail->setStyleSheet(editStyle);

    // 按钮
    QPushButton* btnLogin = new QPushButton("LOGIN");
    btnLogin->setStyleSheet("QPushButton { background-color: gold; color: black; font-weight: bold; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #ffec8b; }");

    QPushButton* btnReg = new QPushButton("REGISTER");
    btnReg->setStyleSheet("QPushButton { background-color: transparent; color: white; border: 1px solid white; padding: 8px; border-radius: 5px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.2); }");

    // 添加到布局
    formLayout->addWidget(title);
    formLayout->addWidget(m_editUser);
    formLayout->addWidget(m_editPass);
    formLayout->addWidget(m_editEmail);
    formLayout->addStretch();
    formLayout->addWidget(btnLogin);
    formLayout->addWidget(btnReg);

    mainLayout->addWidget(container);

    // 连接信号
    connect(btnLogin, &QPushButton::clicked, this, &PageLogin::onLoginClicked);
    connect(btnReg, &QPushButton::clicked, this, &PageLogin::onRegisterClicked);
}

void PageLogin::onLoginClicked() {
    QString user = m_editUser->text();
    QString pass = m_editPass->text();

    if (UserManager::instance().login(user, pass)) {
        // 登录成功，跳转到主菜单
        m_mainWin->switchPage(1);
    }
    else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误.");
    }
}

void PageLogin::onRegisterClicked() {
    QString user = m_editUser->text();
    QString pass = m_editPass->text();
    QString email = m_editEmail->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and Password cannot be empty.");
        return;
    }

    if (UserManager::instance().registerUser(user, pass, email)) {
        QMessageBox::information(this, "Success", "Registration successful! Please login.");
    }
    else {
        QMessageBox::warning(this, "Error", "Username already exists.");
    }
}