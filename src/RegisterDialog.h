#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "Database.h"

class QLineEdit;

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    RegisterDialog(Database* db, QWidget* parent = nullptr);
    QString getRegisteredLogin() const { return m_registeredLogin; }

private slots:
    void onRegister();

private:
    Database* m_db;
    QLineEdit* m_loginEdit;
    QLineEdit* m_passEdit;
    QLineEdit* m_passConfirmEdit;
    QString m_registeredLogin;
};

#endif 
