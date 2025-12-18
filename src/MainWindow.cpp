#include "MainWindow.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QApplication>  

MainWindow::MainWindow(const User& user, QWidget *parent)
    : QMainWindow(parent), m_user(user) {
    m_db.open("threats_manager.db");
    setWindowTitle("Система управления угрозами ИБ — " + QString::fromStdString(m_user.getLogin()));
    resize(900, 600);
    setupUI();
    refreshTable();
}

void MainWindow::setupUI() {
    auto central = new QWidget;
    auto layout = new QVBoxLayout(central);
    setCentralWidget(central);

    m_tableView = new QTableView;
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_tableView);

    // Тулбар
    auto toolbar = addToolBar("Actions");
    toolbar->addAction("Добавить угрозу", this, &MainWindow::addThreat);
    toolbar->addAction("Удалить угрозу", this, &MainWindow::deleteThreat);
    toolbar->addSeparator();
    toolbar->addAction("Статистика", this, &MainWindow::showStats);
    toolbar->addAction("Мои логи", this, &MainWindow::showMyLogs);
    if (m_user.isAdmin()) {
        toolbar->addAction("Системные логи", this, &MainWindow::showAllLogs);
    }
    toolbar->addSeparator();
    toolbar->addAction("Выход", [this]() {
        m_db.logEvent(m_user.getId(), "logout", "Выход из системы");
        qApp->quit();
    });
}

void MainWindow::refreshTable() {
    auto threats = m_db.getAllThreats();
    m_model = new QSqlTableModel(this);
    m_model->setTable("threats");
    m_model->select();

    // Вручную заполним модель (проще, чем QSqlQueryModel)
    if (m_model->rowCount() == 0) {
        // Пока оставим таблицу пустой — заполним вручную
    }

    // Создадим собственную модель
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Название", "Тип", "Риск", "Описание", "Дата"});

    for (const auto& t : threats) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(t.id))
            << new QStandardItem(QString::fromStdString(t.name))
            << new QStandardItem(QString::fromStdString(t.type_name))
            << new QStandardItem(QString::fromStdString(t.risk_level_name))
            << new QStandardItem(QString::fromStdString(t.description))
            << new QStandardItem(QString::fromStdString(t.created_at));
        model->appendRow(row);
    }

    m_tableView->setModel(model);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::addThreat() {
    ThreatForm form(&m_db, this);
    if (form.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void MainWindow::deleteThreat() {
    auto idx = m_tableView->selectionModel()->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите угрозу для удаления!");
        return;
    }
    int row = idx.row();
    int id = m_tableView->model()->index(row, 0).data().toInt();

    if (QMessageBox::question(this, "Подтверждение", "Удалить угрозу ID=" + QString::number(id) + "?") == QMessageBox::Yes) {
        if (m_db.deleteThreat(id)) {
            refreshTable();
            QMessageBox::information(this, "Успех", "Угроза удалена.");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить угрозу.");
        }
    }
}

void MainWindow::showStats() {
    auto s = m_db.getStatistics();
    QString text = QString(
        "<h3>Статистика</h3>"
        "<ul>"
        "<li>Всего угроз: %1</li>"
        "<li>Высокий/Критический риск: %2</li>"
        "<li>Средний риск: %3</li>"
        "<li>Низкий риск: %4</li>"
        ).arg(s.total_threats)
         .arg(s.high_risk_threats)
         .arg(s.medium_risk_threats)
         .arg(s.low_risk_threats);

    if (m_user.isAdmin()) {
        text += QString("<li>Всего пользователей: %1</li><li>Всего логов: %2</li>")
                .arg(s.total_users).arg(s.total_logs);
    }
    text += "</ul>";

    QMessageBox::information(this, "Статистика", text);
}

void MainWindow::showMyLogs() {
    auto logs = m_db.getUserLogs(m_user.getId());
    QString text;
    for (const auto& log : logs) {
        text += QString("[%1] %2: %3\n")
                .arg(QString::fromStdString(log.timestamp))
                .arg(QString::fromStdString(log.event))
                .arg(QString::fromStdString(log.details));
    }
    QMessageBox::information(this, "Мои логи", text.isEmpty() ? "Нет записей" : text);
}

void MainWindow::showAllLogs() {
    if (!m_user.isAdmin()) return;
    auto logs = m_db.getLogs();
    QString text;
    for (const auto& log : logs) {
        text += QString("[%1] %2 (%3): %4\n")
                .arg(QString::fromStdString(log.timestamp))
                .arg(QString::fromStdString(log.user_login))
                .arg(QString::fromStdString(log.event))
                .arg(QString::fromStdString(log.details));
    }
    QMessageBox::information(this, "Системные логи", text.isEmpty() ? "Нет записей" : text);
}
