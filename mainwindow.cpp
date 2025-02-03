#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "createuser.h"
#include "createdevice.h"
#include <QCheckBox>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pb_login,&QPushButton::clicked,this,&MainWindow::tryLogin);
    connect(ui->pb_logout,&QPushButton::clicked,this,&MainWindow::logout);
    connect(ui->pb_addNewUser,&QPushButton::clicked,this,&MainWindow::addNewUser);
    connect(ui->pb_addNewDevice,&QPushButton::clicked,this,&MainWindow::addNewDevice);
    connect(ui->tw_devices, &QTableWidget::cellClicked, this, &MainWindow::TableWidgetRowClicked);
    connect(ui->pb_back,&QPushButton::clicked,this,&MainWindow::showMain);
    connect (ui->pb_change,&QPushButton::clicked,this,&MainWindow::lereadonly);
    connect (ui->pb_ok,&QPushButton::clicked,this,&MainWindow::updateDeviceInfo);
    connect (ui->pb_delete,&QPushButton::clicked,this,&MainWindow::deleteDevice);
    connect(ui->le_fId, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList selectedTypes = getSelectedTypesFromTreeWidget();
        filterTable(selectedTypes);
    });
    connect(ui->le_fModel, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList selectedTypes = getSelectedTypesFromTreeWidget();
        filterTable(selectedTypes);
    });
    connect(ui->le_fManufacturer, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList selectedTypes = getSelectedTypesFromTreeWidget();
        filterTable(selectedTypes);
    });
    connect(ui->le_fRoom, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList selectedTypes = getSelectedTypesFromTreeWidget();
        filterTable(selectedTypes);
    });
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::treeItemClicked);
    connect(ui->pb_clear, &QPushButton::clicked, this, &MainWindow::clearFilters);

    showAuth();

    ui->treeWidget->expandAll();

    ui->tw_devices->setColumnWidth(0,80);
    ui->tw_devices->setColumnWidth(1,300);
    ui->tw_devices->setColumnWidth(2,300);
    ui->tw_devices->setColumnWidth(3,80);
    ui->tw_devices->setColumnWidth(4,250);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::getSelectedTypesFromTreeWidget()
{
    QStringList selectedTypes;
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
        if ((*it)->checkState(0) == Qt::Checked && (*it)->childCount() == 0)
        {
            selectedTypes.append((*it)->text(0));
        }
        ++it;
    }
    return selectedTypes;
}

void MainWindow::showAuth()
{
    ui->frame_auth->setGeometry(ui->frame_auth->x(), 0, ui->frame_auth->width(), ui->frame_auth->height());
    ui->frame_main->setGeometry(ui->frame_main->x(), 1000, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_device->setGeometry(ui->frame_device->x(), 1000, ui->frame_device->width(), ui->frame_device->height());
    ui->frame_deviceDetails->setGeometry(ui->frame_deviceDetails->x(), 1000, ui->frame_deviceDetails->width(), ui->frame_deviceDetails->height());
    ui->frame_auth->setEnabled(true);
    ui->frame_main->setEnabled(false);
    ui->frame_device->setEnabled(false);
    ui->frame_deviceDetails->setEnabled(false);
}

void MainWindow::showMain()
{
    ui->frame_auth->setGeometry(ui->frame_auth->x(), 1000, ui->frame_auth->width(), ui->frame_auth->height());
    ui->frame_main->setGeometry(ui->frame_main->x(), 0, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_device->setGeometry(ui->frame_device->x(), 0, ui->frame_device->width(), ui->frame_device->height());
    ui->frame_deviceDetails->setGeometry(ui->frame_deviceDetails->x(), 1000, ui->frame_deviceDetails->width(), ui->frame_deviceDetails->height());
    ui->frame_auth->setEnabled(false);
    ui->frame_main->setEnabled(true);
    ui->frame_device->setEnabled(true);
    ui->frame_deviceDetails->setEnabled(false);
    loadDeviceTable();
    loadStatuses();
    loadTypes();
    fillTreeWidget();
    curentDeviceID = -1;
    ui->le_model->setReadOnly(true);
    ui->le_manufacturer->setReadOnly(true);
    ui->le_responsible->setReadOnly(true);
    ui->le_room->setReadOnly(true);
    ui->cb_type->setEnabled(false);
    ui->cb_status->setEnabled(false);
    ui->pb_ok->setVisible(false);
}

void MainWindow::tryLogin()
{
    QString login = ui->le_login->text().remove(" ");
    QString password = ui->le_password->text().remove(" ");

    if (login.isEmpty() && password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поля 'Фамилия' и 'Пароль'.");
        return;
    }
    else if (login.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Фамилия'.");
        return;
    }
    else if (password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Пароль'.");
        return;
    }

    db = QSqlDatabase::database("db_sariglar");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT id, role_id FROM users INNER JOIN auth ON users.id = auth.user_id WHERE users.surname = ? AND auth.password = ?;");
        query.addBindValue(login);
        query.addBindValue(password);
        query.exec();
        if(query.next())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Успешная авторизация!", "Успех", QMessageBox::Information, buttons);
            userID = query.value(0).toString();
            userRole = query.value(1).toString();
            ui->le_login->clear();
            ui->le_password->clear();
            showMain();
            adminFunction();
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Пользователя с такими данными не существует!");
            ui->le_login->clear();
            ui->le_password->clear();
            return;
        }
    }
}

bool MainWindow::hasAccess(const QString &requiredRole)
{
    return userRole == requiredRole;
    qDebug() << userRole;
}

void MainWindow::adminFunction()
{
    if(hasAccess("2"))
    {
        ui->pb_addNewUser->setVisible(false);
        ui->pb_addNewDevice->setVisible(false);
        ui->pb_change->setVisible(false);
        ui->pb_delete->setVisible(false);
    }
    else
    {
        ui->pb_addNewUser->setVisible(true);
        ui->pb_addNewDevice->setVisible(true);
        ui->pb_change->setVisible(true);
        ui->pb_delete->setVisible(true);
    }
}

void MainWindow::logout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение выхода", "Вы действительно хотите выйти из учётной записи?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        userID.clear();
        userRole.clear();
        showAuth();
    }
}

void MainWindow::addNewUser()
{
    createuser *cnu = new createuser;
    cnu->setWindowTitle("Создание учётной записи");
    cnu->show();
}

void MainWindow::addNewDevice()
{
    createdevice *cnd = new createdevice;
    cnd->setWindowTitle("Добавление нового устройства");
    cnd->show();
    connect(cnd, &createdevice::deviceAdded, this, [=]()
            {
                cnd->close();
                loadDeviceTable();
            });
}

void MainWindow::loadDeviceTable()
{
    db = QSqlDatabase::database("db_sariglar");
    if(getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);

        query.prepare("SELECT devices.id, devices.model, devices.manufacturer, devices.room, device_types.name FROM devices "
                      "inner join device_types ON devices.type_id = device_types.id");

        if(query.exec())
        {
            ui->tw_devices->setRowCount(0);
            while(query.next())
            {
                ui->tw_devices->insertRow(row);

                int deviceId = query.value(0).toInt();
                QString deviceModel = query.value(1).toString();
                QString deviceManufacturer = query.value(2).toString();
                QString deviceRoom = query.value(3).toString();
                QString devicetype = query.value(4).toString();

                QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(deviceId));
                QTableWidgetItem *modelItem = new QTableWidgetItem(deviceModel);
                QTableWidgetItem *manufacturerItem = new QTableWidgetItem(deviceManufacturer);
                QTableWidgetItem *roomItem = new QTableWidgetItem(deviceRoom);
                QTableWidgetItem *typeItem = new QTableWidgetItem(devicetype);

                ui->tw_devices->setItem(row, 0, idItem);
                ui->tw_devices->setItem(row, 1, modelItem);
                ui->tw_devices->setItem(row, 2, manufacturerItem);
                ui->tw_devices->setItem(row, 3, roomItem);
                ui->tw_devices->setItem(row, 4, typeItem);

                row++;
            }
        }
    }
}

void MainWindow::TableWidgetRowClicked(int row)
{
    QTableWidgetItem *item = ui->tw_devices->item(row, 0);
    if (item)
    {
        int deviceId = item->text().toInt();
        loadDeviceDetails(deviceId);
    }
}

void MainWindow::loadDeviceDetails(int deviceId)
{
    ui->frame_device->setGeometry(ui->frame_device->x(), 1000, ui->frame_device->width(), ui->frame_device->height());
    ui->frame_deviceDetails->setGeometry(ui->frame_deviceDetails->x(), 2, ui->frame_deviceDetails->width(), ui->frame_deviceDetails->height());
    ui->frame_device->setEnabled(false);
    ui->frame_deviceDetails->setEnabled(true);
    curentDeviceID = deviceId;
    db = QSqlDatabase::database("db_sariglar");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT devices.id, devices.model, devices.manufacturer, devices.responsible, devices.room, devices.type_id, devices.status_id FROM devices "
                      "inner join device_types ON devices.type_id = device_types.id "
                      "inner join statuses ON devices.status_id = statuses.id "
                      "where devices.id = :orderID");
        query.bindValue(":orderID", deviceId);
        if(query.exec())
        {
            while(query.next())
            {
                QString deviceId = query.value(0).toString();
                QString deviceModel = query.value(1).toString();
                QString deviceManufacturer = query.value(2).toString();
                QString deviceResponsible = query.value(3).toString();
                QString deviceRoom = query.value(4).toString();
                int deviceType = query.value(5).toInt();
                int deviceStatus = query.value(6).toInt();

                ui->le_model->setText(deviceModel);
                ui->le_manufacturer->setText(deviceManufacturer);
                ui->le_responsible->setText(deviceResponsible);
                ui->le_room->setText(deviceRoom);

                int indexType = ui->cb_type->findData(deviceType);
                if (indexType != -1)
                {
                    ui->cb_type->setCurrentIndex(indexType);
                }

                int indexStatus = ui->cb_status->findData(deviceStatus);
                if (indexStatus != -1)
                {
                    ui->cb_status->setCurrentIndex(indexStatus);
                }
            }
        }
    }
}

void MainWindow::loadStatuses()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_status->clear();
        query.prepare("SELECT id, name FROM statuses");
        if(query.exec())
        {
            while (query.next())
            {
                int statusId = query.value(0).toInt();
                QString statusName = query.value(1).toString();
                ui->cb_status->addItem(statusName, statusId);
            }
        }
    }
}

void MainWindow::loadTypes()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_type->clear();
        query.prepare("SELECT id, name FROM device_types");
        if(query.exec())
        {
            while (query.next())
            {
                int typeId = query.value(0).toInt();
                QString typeName = query.value(1).toString();
                ui->cb_type->addItem(typeName, typeId);
            }
        }
    }
}

void MainWindow::lereadonly()
{
    bool isReadOnly = ui->le_model->isReadOnly();
    bool newReadOnlyState = !isReadOnly;

    ui->le_model->setReadOnly(newReadOnlyState);
    ui->le_manufacturer->setReadOnly(newReadOnlyState);
    ui->le_responsible->setReadOnly(newReadOnlyState);
    ui->le_room->setReadOnly(newReadOnlyState);

    ui->cb_type->setEnabled(!newReadOnlyState);
    ui->cb_status->setEnabled(!newReadOnlyState);

    ui->pb_ok->setVisible(!newReadOnlyState);

    if (newReadOnlyState)
    {
        ui->pb_change->setText("Изменить");
    }
    else
    {
        ui->pb_change->setText("Отмена");
    }
}

void MainWindow::updateDeviceInfo()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение изменений", "Вы действительно хотите изменить данные?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (ui->le_model->isReadOnly()) return;

        int selectedTypeId = ui->cb_type->currentData().toInt();
        int selectedStatusId = ui->cb_status->currentData().toInt(); // Исправлено на cb_status
        db = QSqlDatabase::database("db_sariglar");
        if (getDBConnection(db))
        {
            QSqlQuery query(db);
            query.prepare("UPDATE devices SET model = ?, manufacturer = ?, type_id = ?, status_id = ?, responsible = ?, room = ? WHERE id = ?");
            query.addBindValue(ui->le_model->text());
            query.addBindValue(ui->le_manufacturer->text());
            query.addBindValue(selectedTypeId);
            query.addBindValue(selectedStatusId);
            query.addBindValue(ui->le_responsible->text());
            query.addBindValue(ui->le_room->text());
            query.addBindValue(curentDeviceID);
            if (query.exec())
            {
                QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                showMessage("Данные обновлены!", "Успех", QMessageBox::Information, buttons);
                lereadonly();
                loadDeviceDetails(curentDeviceID);
            }
        }
    }
}

void MainWindow::deleteDevice()
{
    if (getDBConnection(db))
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите удалить данные?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query(db);
            query.prepare("DELETE FROM devices WHERE id = :id");
            query.bindValue(":id", curentDeviceID);
            if (query.exec())
            {
                showMessage("Данные успешно удалены!", "", QMessageBox::Ok, QMessageBox::Information);
                showMain();
            }
        }
    }
}

void MainWindow::clearRowsInTable(QTableWidget *table)
{
    int rowCount = table->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        table->removeRow(0);
    }
}

void MainWindow::filterTable(const QStringList &selectedTypes)
{
    QString ids = ui->le_fId->text().trimmed();
    QString models = ui->le_fModel->text().trimmed();
    QString manufacturers = ui->le_fManufacturer->text().trimmed();
    QString rooms = ui->le_fRoom->text().trimmed();

    if (selectedTypes.isEmpty() && ids.isEmpty() && models.isEmpty() && manufacturers.isEmpty() && rooms.isEmpty())
    {
        clearRowsInTable(ui->tw_devices);
        loadDeviceTable();
        return;
    }

    QString queryStr = "SELECT devices.id, devices.model, devices.manufacturer, devices.room, device_types.name AS type_name "
                       "FROM devices "
                       "LEFT JOIN device_types ON devices.type_id = device_types.id";

    bool firstCondition = true;

    if (!selectedTypes.isEmpty() || !ids.isEmpty() || !models.isEmpty() || !manufacturers.isEmpty() || !rooms.isEmpty())
    {
        queryStr += " WHERE";
    }

    if (!selectedTypes.isEmpty())
    {
        queryStr += " device_types.name IN (";
        for (int i = 0; i < selectedTypes.size(); ++i)
        {
            queryStr += "?";
            if (i < selectedTypes.size() - 1)
            {
                queryStr += ", ";
            }
        }
        queryStr += ")";
        firstCondition = false;
    }

    if (!ids.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " devices.id = ?";
        firstCondition = false;
    }

    if (!models.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " devices.model LIKE ?";
        firstCondition = false;
    }

    if (!manufacturers.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " devices.manufacturer LIKE ?";
        firstCondition = false;
    }

    if (!rooms.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " devices.room LIKE ?";
    }

    clearRowsInTable(ui->tw_devices);
    db = QSqlDatabase::database("db_sariglar");

    QSqlQuery query(db);
    query.prepare(queryStr);

    int bindIndex = 0;

    for (const QString &type : selectedTypes)
    {
        query.bindValue(bindIndex++, type);
    }

    if (!ids.isEmpty())
    {
        query.bindValue(bindIndex++, ids);
    }

    if (!models.isEmpty())
    {
        query.bindValue(bindIndex++, "%" + models + "%");
    }

    if (!manufacturers.isEmpty())
    {
        query.bindValue(bindIndex++, "%" + manufacturers + "%");
    }

    if (!rooms.isEmpty())
    {
        query.bindValue(bindIndex++, "%" + rooms + "%");
    }

    if (!query.exec())
    {
        qDebug() << "Query execution failed:" << query.lastError().text();
        qDebug() << "Query:" << queryStr;
        return;
    }

    ui->tw_devices->setRowCount(0);

    int row = 0;
    while (query.next())
    {
        ui->tw_devices->insertRow(row);

        int deviceId = query.value("id").toInt();
        QString deviceModel = query.value("model").toString();
        QString deviceManufacturer = query.value("manufacturer").toString();
        QString deviceRoom = query.value("room").toString();
        QString deviceType = query.value("type_name").toString();

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(deviceId));
        QTableWidgetItem *modelItem = new QTableWidgetItem(deviceModel);
        QTableWidgetItem *manufacturerItem = new QTableWidgetItem(deviceManufacturer);
        QTableWidgetItem *roomItem = new QTableWidgetItem(deviceRoom);
        QTableWidgetItem *typeItem = new QTableWidgetItem(deviceType);

        ui->tw_devices->setItem(row, 0, idItem);
        ui->tw_devices->setItem(row, 1, modelItem);
        ui->tw_devices->setItem(row, 2, manufacturerItem);
        ui->tw_devices->setItem(row, 3, roomItem);
        ui->tw_devices->setItem(row, 4, typeItem);

        row++;
    }
}

void MainWindow::fillTreeWidget()
{
    ui->treeWidget->clear();

    QTreeWidgetItem *computersCategory = new QTreeWidgetItem(ui->treeWidget);
    computersCategory->setText(0, "Компьютеры и ноутбуки");
    computersCategory->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *desktops = new QTreeWidgetItem(computersCategory);
    desktops->setText(0, "Персональные компьютеры");
    desktops->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *laptops = new QTreeWidgetItem(computersCategory);
    laptops->setText(0, "Ноутбуки");
    laptops->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *monoblocks = new QTreeWidgetItem(computersCategory);
    monoblocks->setText(0, "Моноблоки");
    monoblocks->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *servers = new QTreeWidgetItem(computersCategory);
    servers->setText(0, "Серверы");
    servers->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *peripheralsCategory = new QTreeWidgetItem(ui->treeWidget);
    peripheralsCategory->setText(0, "Периферия");
    peripheralsCategory->setCheckState(0, Qt::Unchecked);

    QStringList peripherals = {
        "Мониторы", "Принтеры", "Сканеры", "МФУ", "Проекторы",
        "Мыши", "Клавиатуры", "Веб-камеры", "Наушники и гарнитуры",
        "Колонки", "Микрофоны", "Видеокамеры", "ИБП"
    };

    for (const QString &item : peripherals)
    {
        QTreeWidgetItem *peripheral = new QTreeWidgetItem(peripheralsCategory);
        peripheral->setText(0, item);
        peripheral->setCheckState(0, Qt::Unchecked);
    }

    QTreeWidgetItem *networkCategory = new QTreeWidgetItem(ui->treeWidget);
    networkCategory->setText(0, "Сетевое оборудование");
    networkCategory->setCheckState(0, Qt::Unchecked);

    QStringList networkDevices = {"Маршрутизаторы", "Коммутаторы"};

    for (const QString &item : networkDevices)
    {
        QTreeWidgetItem *networkDevice = new QTreeWidgetItem(networkCategory);
        networkDevice->setText(0, item);
        networkDevice->setCheckState(0, Qt::Unchecked);
    }

    QTreeWidgetItem *otherCategory = new QTreeWidgetItem(ui->treeWidget);
    otherCategory->setText(0, "Прочее");
    otherCategory->setCheckState(0, Qt::Unchecked);

    QStringList otherItems = {"Интерактивные доски", "3D-принтеры"};

    for (const QString &item : otherItems)
    {
        QTreeWidgetItem *otherItem = new QTreeWidgetItem(otherCategory);
        otherItem->setText(0, item);
        otherItem->setCheckState(0, Qt::Unchecked);
    }

    ui->treeWidget->expandAll();
}

void MainWindow::treeItemClicked(QTreeWidgetItem *item, int column)
{
    if (column == 0 && item->checkState(0) != Qt::Unchecked)
    {
        filterTableByCheckedItems();
    }
    else
    {
        filterTableBySingleItem(item);
    }
}

void MainWindow::filterTableBySingleItem(QTreeWidgetItem *item)
{
    QStringList selectedTypes;
    if (item->childCount() > 0)
    {
        for (int i = 0; i < item->childCount(); ++i)
        {
            selectedTypes.append(item->child(i)->text(0));
        }
    }
    else
    {
        selectedTypes.append(item->text(0));
    }
    filterTable(selectedTypes);
}

void MainWindow::filterTableByCheckedItems()
{
    QStringList selectedTypes;
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
        if ((*it)->checkState(0) == Qt::Checked && (*it)->childCount() == 0)
        {
            selectedTypes.append((*it)->text(0));
        }
        ++it;
    }
    filterTable(selectedTypes);
}

void MainWindow::clearFilters()
{
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
        (*it)->setCheckState(0, Qt::Unchecked);
        ++it;
    }

    ui->le_fId->clear();
    ui->le_fModel->clear();
    ui->le_fManufacturer->clear();
    ui->le_fRoom->clear();

    clearRowsInTable(ui->tw_devices);
    loadDeviceTable();
}
