#include "ThreatForm.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

ThreatForm::ThreatForm(Database* db, QWidget* parent)
    : QDialog(parent), m_db(db) {
    setWindowTitle("Добавить угрозу");
    resize(400, 300);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Название:"));
    m_nameEdit = new QLineEdit;
    layout->addWidget(m_nameEdit);

    layout->addWidget(new QLabel("Тип угрозы:"));
    m_typeCombo = new QComboBox;
    auto types = m_db->getThreatTypes();
    for (const auto& t : types) {
        m_typeCombo->addItem(QString::fromStdString(t.name), t.id);
    }
    layout->addWidget(m_typeCombo);

    layout->addWidget(new QLabel("Уровень риска:"));
    m_riskCombo = new QComboBox;
    auto risks = m_db->getRiskLevels();
    for (const auto& r : risks) {
        m_riskCombo->addItem(QString::fromStdString(r.name), r.id);
    }
    layout->addWidget(m_riskCombo);

    layout->addWidget(new QLabel("Описание:"));
    m_descEdit = new QTextEdit;
    layout->addWidget(m_descEdit);

    auto btns = new QHBoxLayout;
    auto saveBtn = new QPushButton("Сохранить");
    auto cancelBtn = new QPushButton("Отмена");
    btns->addWidget(saveBtn);
    btns->addWidget(cancelBtn);
    layout->addLayout(btns);

    connect(saveBtn, &QPushButton::clicked, this, &ThreatForm::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ThreatForm::onSave() {
    QString name = m_nameEdit->text().trimmed();
    QString desc = m_descEdit->toPlainText();
    int typeId = m_typeCombo->currentData().toInt();
    int riskId = m_riskCombo->currentData().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите название угрозы!");
        return;
    }

    Threat t;
    t.name = name.toStdString();
    t.type_id = typeId;
    t.risk_level_id = riskId;
    t.description = desc.toStdString();

    if (m_db->addThreat(t)) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить угрозу!");
    }
}
