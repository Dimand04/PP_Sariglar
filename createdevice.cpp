#include "createdevice.h"
#include "ui_createdevice.h"
#include "global.h"

createdevice::createdevice(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createdevice)
{
    ui->setupUi(this);
    connect (ui->pb_addNewDevice,&QPushButton::clicked,this,&createdevice::createNewDevice);
    loadTypes();
}

createdevice::~createdevice()
{
    delete ui;
}

void createdevice::loadTypes()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_types->clear();
        query.prepare("SELECT id, name FROM device_types");
        if(query.exec())
        {
            while (query.next()) {
                int typeId = query.value(0).toInt();
                QString typeName = query.value(1).toString();
                ui->cb_types->addItem(typeName, typeId);
            }
        }
    }
}

void createdevice::createNewDevice()
{
    if (ui->le_model->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле модель не может быть пустым!");
        return;
    }

    if (ui->le_manufacturer->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле поставщик не может быть пустым!");
        return;
    }

    if (ui->le_responsible->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле ответственный не может быть пустым!");
        return;
    }

    if (ui->le_room->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле кабинет не может быть пустым!");
        return;
    }

    if (ui->cb_types->currentIndex() == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите тип из списка!");
        return;
    }

    int selectedTypeId = ui->cb_types->currentData().toInt();
    db = QSqlDatabase::database("db_sariglar");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO devices (model, manufacturer, type_id, status_id, responsible, room) values (?, ?, ?, ?, ?, ?);");
        query.addBindValue(ui->le_model->text());
        query.addBindValue(ui->le_manufacturer->text());
        query.addBindValue(selectedTypeId);
        query.addBindValue(1);
        query.addBindValue(ui->le_responsible->text());
        query.addBindValue(ui->le_room->text());
        if (query.exec())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Устройство успешно добавлено!", "", QMessageBox::Information, buttons);
            ui->le_model->clear();
            ui->le_manufacturer->clear();
            ui->le_responsible->clear();
            ui->le_room->clear();
            emit deviceAdded();
        }
    }
}
