#include "createuser.h"
#include "ui_createuser.h"
#include "global.h"

createuser::createuser(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createuser)
{
    ui->setupUi(this);
    connect (ui->qpb_createNewAccount,&QPushButton::clicked,this,&createuser::createNewAccount);
    loadRoles();
    setupValidators();
}

createuser::~createuser()
{
    delete ui;
}

void createuser::loadRoles()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_roles->clear();
        query.prepare("SELECT id, name FROM roles");
        if(query.exec())
        {
            while (query.next()) {
                int faceId = query.value(0).toInt();
                QString faceName = query.value(1).toString();
                ui->cb_roles->addItem(faceName, faceId);
            }
        }
    }
}

void createuser::createNewAccount()
{
    if (ui->le_surname->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле фамилии не может быть пустыми!");
        return;
    }

    if (ui->le_password->text().isEmpty() || ui->le_confirmPassword->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пароль и подтверждение пароля не могут быть пустыми!");
        return;
    }

    if (ui->le_password->text() != ui->le_confirmPassword->text()) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают! Проверьте введенные данные.");
        return;
    }

    if (ui->cb_roles->currentIndex() == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите роль из списка!");
        return;
    }

    int selectedRoleId = ui->cb_roles->currentData().toInt();
    db = QSqlDatabase::database("db_sariglar");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO users (surname, role_id) VALUES (?, ?);");
        query.addBindValue(ui->le_surname->text());
        query.addBindValue(selectedRoleId);

        if (query.exec())
        {
            query.exec("SELECT MAX(id) FROM users;");
            if (query.next())
            {
                userID = query.value(0).toString();
                query.prepare("INSERT INTO auth (user_id, password) VALUES (?, ?);");
                query.addBindValue(userID);
                query.addBindValue(ui->le_password->text());
                if (query.exec())
                {
                    QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                    showMessage("Успешная регистрация!", "", QMessageBox::Information, buttons);
                    ui->le_surname->clear();
                    ui->le_password->clear();
                    ui->le_confirmPassword->clear();
                    close();
                }
            }
        }
    }
}

void createuser::setupValidators()
{
    QRegularExpression textRegex("^[А-Яа-яA-Za-z ]+$");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(textRegex, this);

    ui->le_surname->setValidator(textValidator);
}
