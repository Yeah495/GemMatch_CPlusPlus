#include "PageLogin.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QDebug>

PageLogin::PageLogin(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}



void PageLogin::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. View & Scene
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // 2. 视频层 (Z=0)
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);
    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800)); // 初始大小，会被 resizeEvent 覆盖
    m_videoItem->setZValue(0);
    scene->addItem(m_videoItem);
    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/1.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // =========================================================
    // 3. 登录框容器 (不包含 Logo 了)
    // =========================================================
    QWidget* loginContainer = new QWidget();
    loginContainer->setFixedSize(500, 400); // 高度减小了，因为移走了Logo
    loginContainer->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 50);" /* 白色，70%不透明 */
        "   border-radius: 20px;"                        /* 更大的圆角 */
        "   border: 1px solid rgba(255, 255, 255, 200);" /* 亮边框，增加立体感 */
        "}"
    );
    QVBoxLayout* formLayout = new QVBoxLayout(loginContainer);
    formLayout->setContentsMargins(30, 30, 30, 30);
    formLayout->setSpacing(15);
    formLayout->setAlignment(Qt::AlignHCenter);

    // 输入框样式
    QString editStyle =
        "QLineEdit {"
        "   background-color: rgba(255, 255, 255, 200);" /* 比容器更透一点 */
        "   font-weight: bold;"          /* <--- 添加这一行：字体加粗 */
        "   border: none;"
        "   font-family: 'Microsoft YaHei';" /* (可选) 设置一种好看的字体，如微软雅黑 */
        "   border-radius: 15px;"         /* 纯圆角 */
        "   padding: 10px 15px;"          /* 内部文字留出呼吸空间 */
        "   font-size: 16px;"
        "   color: #044BB7;"              /* 深蓝灰色字体，不要用纯黑 */
        "   selection-background-color: #044BB7;"
        "}"
        "QLineEdit:focus {"
        "   background-color: rgba(255, 255, 255, 220);" /* 聚焦时变亮 */
        "   border: 2px solid #00BFFF;"   /* 聚焦时显示宝石蓝边框 */
        "}";

    m_editUser = new QLineEdit();
    m_editUser->setPlaceholderText("用户名");
    m_editUser->setStyleSheet(editStyle);
    m_editUser->setFixedWidth(260);

    m_editPass = new QLineEdit();
    m_editPass->setPlaceholderText("密码");
    m_editPass->setEchoMode(QLineEdit::Password);
    m_editPass->setStyleSheet(editStyle);
    m_editPass->setFixedWidth(260);

    m_editEmail = new QLineEdit();
    m_editEmail->setPlaceholderText("邮箱");
    m_editEmail->setStyleSheet(editStyle);
    m_editEmail->setFixedWidth(260);

    m_btnLogin = new GameButton("assets/images/登录.png");
    m_btnReg = new GameButton("assets/images/注册.png");

    // 添加控件到登录框 (注意：没有 Logo)
    formLayout->addWidget(m_editUser);
    formLayout->addWidget(m_editPass);
    formLayout->addWidget(m_editEmail);
    formLayout->addStretch();
    formLayout->addWidget(m_btnLogin, 0, Qt::AlignHCenter);
    formLayout->addWidget(m_btnReg, 0, Qt::AlignHCenter);

    // 将登录框加入场景 (Z=1)
    m_loginBoxProxy = scene->addWidget(loginContainer);
    m_loginBoxProxy->setZValue(1);

    // =========================================================
    // 4. Logo 独立容器
    // =========================================================
    // 创建 Logo 对象
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");

    // 直接将 Logo 加入场景，获得独立的 Proxy (Z=2，保证在登录框上方或同层)
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

    // 添加 View 到主布局
    mainLayout->addWidget(m_view);

    // 连接信号
    connect(m_btnLogin, &QPushButton::clicked, this, &PageLogin::onLoginClicked);
    connect(m_btnReg, &QPushButton::clicked, this, &PageLogin::onRegisterClicked);
}

void PageLogin::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        // 1. 背景视频全屏
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 2. 重新定位 登录框 (LoginBox) -> 屏幕正中央稍偏下
        if (m_loginBoxProxy) {
            qreal boxW = m_loginBoxProxy->widget()->width();
            qreal boxH = m_loginBoxProxy->widget()->height();
            // 居中公式：(屏幕宽 - 控件宽)/2
            // Y轴位置：屏幕高度 * 0.6 (放在下半部分)
            m_loginBoxProxy->setPos((this->width() - boxW) / 2,
                (this->height() - boxH) / 2 + 50);
        }

        // 3. 重新定位 Logo -> 屏幕正中央稍偏上
        if (m_logoProxy) {
            qreal logoW = m_logoProxy->widget()->width();
            // Y轴位置：屏幕高度 * 0.10 (放在上面)
            m_logoProxy->setPos((this->width() - logoW) / 2,
                this->height() * 0.10);
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




void PageLogin::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // 页面隐藏时，暂停播放以释放CPU/GPU资源
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 关键！设为空，强制释放视频占用的内存

    }
}


void PageLogin::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // 1. 播放视频
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }

    // 2. 触发 Logo 掉落动画
    if (m_logo) {
        m_logo->startEntrance();
    }
}




//    // 新增调试：鼠标点击事件
void PageLogin::mousePressEvent(QMouseEvent* event) {
    // 获取点击在窗口中的坐标
    QPoint viewPos = event->pos();

    // 如果需要场景坐标（通常和窗口坐标一致，除非你做了视口缩放）
    QPointF scenePos = m_view->mapToScene(viewPos);

    qDebug() << "========================================";
    qDebug() << "【当前点击坐标】 X:" << viewPos.x() << "  Y:" << viewPos.y();
    qDebug() << "  -> 建议代码: setPos(" << viewPos.x() << "," << viewPos.y() << ");";
    qDebug() << "========================================";

    QWidget::mousePressEvent(event); // 传递事件，不影响正常点击
}