#include "RegisterDialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

RegisterDialog::RegisterDialog(Database* db, QWidget* parent)
    : QDialog(parent), m_db(db) {
    setWindowTitle("Регистрация");
    setFixedSize(300, 220);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Логин:"));
    m_loginEdit = new QLineEdit;
    layout->addWidget(m_loginEdit);

    layout->addWidget(new QLabel("Пароль (мин. 4 символа):"));
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passEdit);

    layout->addWidget(new QLabel("Подтверждение пароля:"));
    m_passConfirmEdit = new QLineEdit;
    m_passConfirmEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passConfirmEdit);

    auto btnLayout = new QHBoxLayout;
    auto regBtn = new QPushButton("Зарегистрироваться");
    auto cancelBtn = new QPushButton("Отмена");
    btnLayout->addWidget(regBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(regBtn, &QPushButton::clicked, this, &RegisterDialog::onRegister);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::onRegister() {
    QString login = m_loginEdit->text().trimmed();
    QString pass = m_passEdit->text();
    QString confirm = m_passConfirmEdit->text();

    if (login.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Логин не может быть пустым!");
        return;
    }
    if (pass.length() < 4) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 4 символов!");
        return;
    }
    if (pass != confirm) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают!");
        return;
    }

    if (m_db->userExists(login.toStdString())) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким логином уже существует!");
        return;
    }

    if (m_db->registerUser(login.toStdString(), pass.toStdString(), "user")) {
        m_registeredLogin = login;
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось зарегистрировать пользователя!");
    }
}
