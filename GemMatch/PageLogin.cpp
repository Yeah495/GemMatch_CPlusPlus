#include "PageLogin.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

PageLogin::PageLogin(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageLogin::setupUI() {
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========== 步骤 1: 创建 Graphics View ==========
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // ========== 步骤 2: 添加视频层（底层，Z=0）==========
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));
    m_videoItem->setZValue(0);  // 底层
    scene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);

    m_videoPath = "assets/videos/1.mp4"; // 注意：PageLogin用1.mp4, SceneGame用4.mp4
    m_player->setLoops(QMediaPlayer::Infinite);

    m_player->setLoops(QMediaPlayer::Infinite);


    // ========== 步骤 3: 创建 UI 容器（顶层）==========
    QWidget* container = new QWidget();
    container->setFixedSize(320, 450);
    container->setStyleSheet(
        "QWidget { background-color: rgba(0, 0, 0, 180); border-radius: 15px; }"
    );

    QVBoxLayout* formLayout = new QVBoxLayout(container);
    formLayout->setContentsMargins(30, 40, 30, 40);
    formLayout->setSpacing(15);

    // 标题
    QLabel* title = new QLabel("宝石迷阵");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: gold; font-size: 28px; font-weight: bold;");

    // 输入框
    QString editStyle =
        "QLineEdit { padding: 8px; border-radius: 5px; border: 1px solid #555; "
        "background: #333; color: white; } "
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
    btnLogin->setStyleSheet(
        "QPushButton { background-color: gold; color: black; font-weight: bold; "
        "padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #ffec8b; }"
    );

    QPushButton* btnReg = new QPushButton("注册");
    btnReg->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; "
        "border: 1px solid white; padding: 8px; border-radius: 5px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.2); }"
    );

    formLayout->addWidget(title);
    formLayout->addWidget(m_editUser);
    formLayout->addWidget(m_editPass);
    formLayout->addWidget(m_editEmail);
    formLayout->addStretch();
    formLayout->addWidget(btnLogin);
    formLayout->addWidget(btnReg);

    // ========== 步骤 4: 将容器添加到场景（Z=1，在视频上方）==========
    QGraphicsProxyWidget* proxy = scene->addWidget(container);
    proxy->setZValue(1);  // 顶层

    // 居中定位
    proxy->setPos((2560 - 320) / 2, (1600 - 450) / 2);

    // 添加到主布局
    mainLayout->addWidget(m_view);

    // 连接信号
    connect(btnLogin, &QPushButton::clicked, this, &PageLogin::onLoginClicked);
    connect(btnReg, &QPushButton::clicked, this, &PageLogin::onRegisterClicked);
}

void PageLogin::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 重新居中容器
        QList<QGraphicsItem*> items = m_view->scene()->items();
        for (auto* item : items) {
            if (QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item)) {
                QWidget* widget = proxy->widget();
                if (widget) {
                    proxy->setPos((this->width() - widget->width()) / 2,
                        (this->height() - widget->height()) / 2);
                }
            }
        }
    }
}

void PageLogin::onLoginClicked() {
    QString user = m_editUser->text();
    QString pass = m_editPass->text();

    if (UserManager::instance().login(user, pass)) {
        m_mainWin->switchPage(1);
    }
    else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
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
        QMessageBox::warning(this, "错误", "该用户名已存在");
    }
}


void PageLogin::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 页面显示时，开始播放
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath)); // ✅ 此时才加载进内存
        m_player->play();
    }
}

void PageLogin::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // 页面隐藏时，暂停播放以释放CPU/GPU资源
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 关键！设为空，强制释放视频占用的内存

    }
}