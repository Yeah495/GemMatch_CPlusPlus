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
#include "Email.h"
#include <QInputDialog>     
#include <QRandomGenerator>



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
        "   background-color: rgba(255, 255, 255, 100);" /* 白色，70%不透明 */
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
        "   background-color: rgba(255, 255, 255, 230);" /* 比容器更透一点 */
        "   font-weight: bold;"          /* <--- 添加这一行：字体加粗 */
        "   border: none;"
        "   font-family: 'Microsoft YaHei';" /* (可选) 设置一种好看的字体，如微软雅黑 */
        "   border-radius: 15px;"         /* 纯圆角 */
        "   padding: 10px 15px;"          /* 内部文字留出呼吸空间 */
        "   font-size: 20px;"
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
    m_editUser->setFixedWidth(350);

    m_editPass = new QLineEdit();
    m_editPass->setPlaceholderText("密码");
    m_editPass->setEchoMode(QLineEdit::Password);
    m_editPass->setStyleSheet(editStyle);
    m_editPass->setFixedWidth(350);

    m_editEmail = new QLineEdit();
    m_editEmail->setPlaceholderText("邮箱");
    m_editEmail->setStyleSheet(editStyle);
    m_editEmail->setFixedWidth(350);

    m_btnLogin = new GameButton("assets/images/登录1.png");
    m_btnReg = new GameButton("assets/images/注册1.png");

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

    // =========================================================
    // 【新增】 5. 左下角后台按键
    // =========================================================
    m_btnBackstage = new GameButton("assets/images/后台.png");
    // 如果图片太大，可以强制缩小一点，例如：
    // m_btnBackstage->setFixedSize(60, 60); 

    m_backstageProxy = scene->addWidget(m_btnBackstage);
    m_backstageProxy->setZValue(2); // 确保在最上层

    // 修改 PageLogin.cpp 中的后台按钮连接代码
    connect(m_btnBackstage, &QPushButton::clicked, this, [this]() {
        // 创建管理员密码验证对话框
        QDialog passwordDialog(this);
        passwordDialog.setWindowTitle("🔐 管理员身份验证");
        passwordDialog.setFixedSize(400, 200);

        // 设置对话框样式
        passwordDialog.setStyleSheet(
            "QDialog {"
            "   background-color: #1a1a1a;"  // 深色背景
            "}"
            "QLabel {"
            "   color: white;"               // 白色字体
            "   font-size: 14px;"
            "}"
            "QLineEdit {"
            "   background-color: #333333;"  // 深灰色输入框
            "   color: white;"               // 白色字体
            "   border: 2px solid #444444;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: 14px;"
            "}"
            "QLineEdit:focus {"
            "   border-color: #3498db;"      // 聚焦时边框颜色
            "}"
            "QPushButton {"
            "   background-color: #333333;"  // 深灰色按钮
            "   color: white;"
            "   border: 2px solid #444444;"
            "   border-radius: 8px;"
            "   padding: 10px 20px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "   min-width: 100px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #444444;"  // 悬停时稍亮
            "   border-color: #555555;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #222222;"  // 按下时更深
            "}"
            "QPushButton#confirmBtn {"
            "   background-color: #27ae60;"  // 确认按钮绿色
            "   border-color: #2ecc71;"
            "}"
            "QPushButton#confirmBtn:hover {"
            "   background-color: #2ecc71;"
            "}"
            "QPushButton#cancelBtn {"
            "   background-color: #c0392b;"  // 取消按钮红色
            "   border-color: #e74c3c;"
            "}"
            "QPushButton#cancelBtn:hover {"
            "   background-color: #e74c3c;"
            "}"
        );

        QVBoxLayout* mainLayout = new QVBoxLayout(&passwordDialog);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(20, 20, 20, 20);

        // 标题
        QLabel* titleLabel = new QLabel("🔐 管理员身份验证", &passwordDialog);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
        titleLabel->setAlignment(Qt::AlignCenter);

        // 说明文字
        QLabel* instructionLabel = new QLabel("请输入管理员密码以进入后台管理系统", &passwordDialog);
        instructionLabel->setAlignment(Qt::AlignCenter);
        instructionLabel->setWordWrap(true);

        // 密码输入框
        QLabel* passwordLabel = new QLabel("管理员密码:", &passwordDialog);
        QLineEdit* passwordEdit = new QLineEdit(&passwordDialog);
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText("请输入管理员密码...");

 

        // 按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* confirmButton = new QPushButton("确定", &passwordDialog);
        confirmButton->setObjectName("confirmBtn");
        QPushButton* cancelButton = new QPushButton("取消", &passwordDialog);
        cancelButton->setObjectName("cancelBtn");

        buttonLayout->addWidget(confirmButton);
        buttonLayout->addWidget(cancelButton);

        // 布局
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(instructionLabel);
        mainLayout->addWidget(passwordLabel);
        mainLayout->addWidget(passwordEdit);
        mainLayout->addStretch();
        mainLayout->addLayout(buttonLayout);

        // 连接按钮信号
        QObject::connect(confirmButton, &QPushButton::clicked, &passwordDialog, &QDialog::accept);
        QObject::connect(cancelButton, &QPushButton::clicked, &passwordDialog, &QDialog::reject);

        // 处理验证结果
        if (passwordDialog.exec() == QDialog::Accepted) {
            QString inputPassword = passwordEdit->text();

            // 验证密码（这里使用简单的固定密码，实际应用中可以改为数据库验证或其他方式）
            if (inputPassword == "admin123") { // 这里可以改为你想要的密码
                // 密码正确，切换到后台管理页面
                if (m_mainWin) {
                    m_mainWin->switchPage(6); // 切换到PageAdmin页面
                }
            }
            else {
                // 密码错误提示
                QMessageBox errorBox;
                errorBox.setWindowTitle("❌ 验证失败");
                errorBox.setText("管理员密码错误！");
                errorBox.setIcon(QMessageBox::Critical);
                errorBox.setStyleSheet(
                    "QMessageBox {"
                    "   background-color: black;"
                    "}"
                    "QMessageBox QLabel {"
                    "   color: white;"
                    "   font-size: 14px;"
                    "}"
                    "QMessageBox QPushButton {"
                    "   background-color: #444444;"
                    "   color: white;"
                    "   border: 1px solid #666666;"
                    "   border-radius: 5px;"
                    "   padding: 8px 16px;"
                    "   min-width: 80px;"
                    "}"
                    "QMessageBox QPushButton:hover {"
                    "   background-color: #555555;"
                    "}"
                );
                errorBox.exec();
            }
        }
        });


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

    if (user.isEmpty() && pass.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }
    else if (user.isEmpty() ) {
        QMessageBox::warning(this, "错误", "用户名不能为空");
        return;
    }
    else if (pass.isEmpty()) {
        QMessageBox::warning(this, "错误", "密码不能为空");
        return;
    }
   

    if (UserManager::instance().login(user, pass)) {
        m_mainWin->switchPage(1);
    }
    else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}


void PageLogin::onRegisterClicked() {
    QString user = m_editUser->text().trimmed();
    QString pass = m_editPass->text().trimmed();
    QString email = m_editEmail->text().trimmed();

    // 1. 基础校验
    if (user.isEmpty() || pass.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名、密码和邮箱不能为空");
        return;
    }

    // 2. 简单的邮箱格式校验（可选）
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "错误", "请输入有效的邮箱地址");
        return;
    }

    // 3. 生成 6 位随机验证码
    QString code = QString::number(QRandomGenerator::global()->bounded(100000, 999999));

    // 4. 配置发件人信息 (⚠️注意：这里需要填你自己的邮箱和授权码)
    // 建议使用 163邮箱 或 QQ邮箱，开启 POP3/SMTP 服务获取"授权码"
    QString senderEmail = "2546696700@qq.com"; // 【请修改这里】你的发送邮箱
    QString senderPass = "ylujglbywznidjih";     // 【请修改这里】邮箱授权码 (不是登录密码!)
    QString smtpHost = "smtp.qq.com";          // QQ邮箱是 smtp.qq.com, 163是 smtp.163.com

    // 5. 发送邮件
    // 提示：发送邮件是异步网络操作，为了简单起见，我们这里直接发送并在下方弹窗等待用户输入
    // 实际项目中可以加一个"发送中..."的等待动画
    Smtp* smtp = new Smtp(senderEmail, senderPass, smtpHost);
    QString subject = "【GenMatch】注册验证码";
    QString body = QString("亲爱的用户 <b>%1</b>：<br>您的注册验证码是：<h2 style='color:blue'>%2</h2><br>请在界面输入此验证码完成注册。")
        .arg(user).arg(code);

    smtp->sendMail(email, subject, body);

    // 6. 弹窗让用户输入验证码
    bool ok;
    QString text = QInputDialog::getText(this, "邮箱验证",
        QString("验证码已发送至 %1\n请查收并输入验证码：").arg(email),
        QLineEdit::Normal,
        "", &ok);

    // 7. 验证逻辑
    if (ok && !text.isEmpty()) {
        if (text.trimmed() == code) {
            // 验证码正确，执行数据库注册
            if (UserManager::instance().registerUser(user, pass, email)) {
                QMessageBox::information(this, "注册成功", "注册成功，请登录");
                // 自动填入用户名，方便登录
                m_editUser->setText(user);
                m_editPass->setText("");
            }
            else {
                QMessageBox::warning(this, "错误", "该用户名已存在");
            }
        }
        else {
            QMessageBox::warning(this, "验证失败", "验证码错误！");
        }
    }

    // 清理 smtp 对象（Smtp类需要完善内存管理，这里为了简单直接new了，建议Smtp内部设为自动deleteLater）
    // 或者将 smtp 设为成员变量管理
}

//void PageLogin::onRegisterClicked() {
//    QString user = m_editUser->text();
//    QString pass = m_editPass->text();
//    QString email = m_editEmail->text();
//
//    if (user.isEmpty()&& pass.isEmpty()&& email.isEmpty()) {
//        QMessageBox::warning(this, "错误", "用户名,密码和邮箱不能为空");
//        return;
//    }
//    else if(user.isEmpty() && pass.isEmpty() ) {
//        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
//        return;
//    }
//    else if (user.isEmpty() && email.isEmpty()) {
//        QMessageBox::warning(this, "错误", "用户名和邮箱不能为空");
//        return;
//    }
//    else if (email.isEmpty() && pass.isEmpty()) {
//        QMessageBox::warning(this, "错误", "密码和邮箱不能为空");
//        return;
//    }
//    else if (user.isEmpty() ) {
//        QMessageBox::warning(this, "错误", "用户名不能为空");
//        return;
//    }
//    else if (pass.isEmpty()) {
//        QMessageBox::warning(this, "错误", "密码不能为空");
//        return;
//    }
//    else if (email.isEmpty()) {
//        QMessageBox::warning(this, "错误", "邮箱不能为空");
//        return;
//    }
//
//    if (UserManager::instance().registerUser(user, pass, email)) {
//        QMessageBox::information(this, "注册成功", "注册成功，请登录");
//    }
//    else {
//        QMessageBox::warning(this, "错误", "该用户名已存在");
//    }
//}




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