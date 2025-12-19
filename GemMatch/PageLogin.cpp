#include "PageLogin.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <QPainter>
#include <QStyleOption>

PageLogin::PageLogin(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageLogin::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    // 如果 init 报错，请使用 initFrom
    opt.initFrom(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PageLogin::setupUI() {
    // 1. 给当前窗口起一个唯一的 ID 名字
    this->setObjectName("LoginBackground");

    // 2.
    // 增加冒号前缀，并匹配 qrc 生成的冗余路径
    this->setStyleSheet("#LoginBackground { border-image: url(:/assets/images/login.jpg); }");

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
    QLabel* title = new QLabel("宝石迷阵");
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
    QPushButton* btnLogin = new QPushButton("登录");
    btnLogin->setStyleSheet("QPushButton { background-color: gold; color: black; font-weight: bold; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #ffec8b; }");

    QPushButton* btnReg = new QPushButton("注册");
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
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }

    if (UserManager::instance().registerUser(user, pass, email)) {
        QMessageBox::information(this, "注册成功", "注册成功，请登录");
    }
    else {
        QMessageBox::warning(this, "错误", "该用户名已存在，请更换");
    }
}