#include "AuthDialog.h"
#include "RegisterDialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

AuthDialog::AuthDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Вход в систему");
    setFixedSize(300, 200);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Логин:"));
    m_loginEdit = new QLineEdit;
    layout->addWidget(m_loginEdit);

    layout->addWidget(new QLabel("Пароль:"));
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);

    auto buttons = new QHBoxLayout;
    auto loginBtn = new QPushButton("Войти");
    auto regBtn = new QPushButton("Регистрация");
    buttons->addWidget(loginBtn);
    buttons->addWidget(regBtn);
    layout->addLayout(buttons);

    connect(loginBtn, &QPushButton::clicked, this, &AuthDialog::onLogin);
    connect(regBtn, &QPushButton::clicked, this, &AuthDialog::onRegister);

    m_db.open("threats_manager.db");
}

void AuthDialog::onLogin() {
    QString login = m_loginEdit->text().trimmed();
    QString pass = m_passwordEdit->text();

    if (login.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }

    m_user = m_db.loginUser(login.toStdString(), pass.toStdString());
    if (m_user.getId() > 0) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль!");
    }
}

void AuthDialog::onRegister() {
    RegisterDialog reg(&m_db, this);
    if (reg.exec() == QDialog::Accepted) {
        m_loginEdit->setText(reg.getRegisteredLogin());
        m_passwordEdit->clear();
    }
}
