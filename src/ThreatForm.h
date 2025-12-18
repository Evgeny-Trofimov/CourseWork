#ifndef THREATFORM_H
#define THREATFORM_H

#include <QDialog>
#include "Database.h"

class QComboBox;
class QLineEdit;
class QTextEdit;

class ThreatForm : public QDialog {
    Q_OBJECT

public:
    ThreatForm(Database* db, QWidget* parent = nullptr);

private slots:
    void onSave();

private:
    Database* m_db;
    QLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_riskCombo;
    QTextEdit* m_descEdit;
};

#endif // THREATFORM_H
