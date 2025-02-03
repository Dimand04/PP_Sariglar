#ifndef CREATEDEVICE_H
#define CREATEDEVICE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class createdevice;
}

class createdevice : public QWidget
{
    Q_OBJECT

signals:
    void deviceAdded();

public:
    explicit createdevice(QWidget *parent = nullptr);
    ~createdevice();

public slots:
    void loadTypes();
    void createNewDevice();

private:
    Ui::createdevice *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_sariglar");
};

#endif // CREATEDEVICE_H
