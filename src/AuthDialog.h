#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "Database.h"

class AuthDialog : public QDialog {
    Q_OBJECT

public:
    AuthDialog(QWidget *parent = nullptr);
    User getCurrentUser() const { return m_user; }

private slots:
    void onLogin();
    void onRegister();

private:
    QLineEdit *m_loginEdit;
    QLineEdit *m_passwordEdit;
    User m_user;
    Database m_db;
};

#endif 
