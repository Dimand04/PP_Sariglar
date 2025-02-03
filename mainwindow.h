#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showAuth();
    void tryLogin();
    void showMain();
    void adminFunction();
    void logout();
    void addNewUser();
    void addNewDevice();
    void loadDeviceTable();
    void TableWidgetRowClicked(int row);
    void loadDeviceDetails(int deviceID);
    void loadStatuses();
    void loadTypes();
    void lereadonly();
    void deleteDevice();
    void filterTable(const QStringList &selectedTypes);
    void clearRowsInTable(QTableWidget*);
    void fillTreeWidget();
    void treeItemClicked(QTreeWidgetItem *item, int column);
    void filterTableBySingleItem(QTreeWidgetItem *item);
    void filterTableByCheckedItems();
    QStringList getSelectedTypesFromTreeWidget();
    void clearFilters();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_sariglar");
    QString userID;
    QString userRole;
    int curentDeviceID;
    bool hasAccess(const QString &requiredRole);
    void updateDeviceInfo();
};
#endif // MAINWINDOW_H
