#ifndef PAGEADMIN_H
#define PAGEADMIN_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QInputDialog>

class MainWindow;

class PageAdmin : public QWidget
{
    Q_OBJECT

public:
    explicit PageAdmin(MainWindow* mainWin);
    ~PageAdmin();

public slots:
    void loadUserData();                    // 加载用户数据
    void searchUsers();                     // 搜索用户
    void clearSearch();                     // 清除搜索
    void deleteSelectedUser();              // 删除选中的用户
    void deleteAllUsers();                  // 删除所有用户
    void resetUserPassword();               // 重置用户密码
    void exportToCSV();                     // 导出为CSV文件
    void refreshTable();                    // 刷新表格
    void onCellClicked(int row, int column); // 单元格点击事件
    void showUserDetails();                 // 显示用户详情

private:
    void setupUI();                         // 初始化UI
    void setupTable();                      // 设置表格
    void setupDatabase();                   // 设置数据库连接
    QList<QStringList> getAllUsers();       // 获取所有用户数据
    bool deleteUser(const QString& username); // 删除用户
    bool resetPassword(const QString& username, const QString& newPassword); // 重置密码
    void showStatistics();                  // 显示统计信息

private:
    MainWindow* m_mainWin;

    // 主布局
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    // UI控件
    QLabel* m_titleLabel;
    QLabel* m_searchLabel;
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QCheckBox* m_showInactiveCheck;

    // 按钮
    QPushButton* m_btnSearch;
    QPushButton* m_btnClear;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnDelete;
    QPushButton* m_btnDeleteAll;
    QPushButton* m_btnResetPwd;
    QPushButton* m_btnExport;
    QPushButton* m_btnStats;
    QPushButton* m_btnBack;

    // 表格
    QTableWidget* m_table;

    // 详情面板
    QWidget* m_detailPanel;
    QVBoxLayout* m_detailLayout;
    QLabel* m_detailUsername;
    QLabel* m_detailEmail;
    QLabel* m_detailEasyScore;
    QLabel* m_detailNormalScore;
    QLabel* m_detailHardScore;
    QLabel* m_detailCreatedAt;

    // 数据库
    QSqlDatabase m_db;
    QString m_currentSelectedUser; // 当前选中的用户名

    QMessageBox* createStyledMessageBox(const QString& title, const QString& text,
        QMessageBox::Icon icon = QMessageBox::Information);
};

#endif // PAGEADMIN_H