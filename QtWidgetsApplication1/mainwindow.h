#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenDatabase();
    void onRefreshData();
    void onExecuteSQL();
    void onExportData();
    void onAddUser();
    void onDeleteUser();
    void onFilterChanged();
    void onShowStats();
    void onBackupDatabase();

private:
    Ui::MainWindow* ui;
    QSqlDatabase m_db;
    QSqlTableModel* m_model;
    QSortFilterProxyModel* m_proxyModel;

    void setupDatabase();
    void setupTableView();
    void loadDatabase(const QString& path);
    void updateStatusBar();
    void executeCustomQuery(const QString& query);
    bool confirmAction(const QString& title, const QString& message);
};
#endif // MAINWINDOW_H