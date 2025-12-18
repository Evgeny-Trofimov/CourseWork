#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QSqlTableModel>
#include "Database.h"
#include "ThreatForm.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(const User& user, QWidget *parent = nullptr);

private slots:
    void addThreat();
    void deleteThreat();
    void showMyLogs();
    void showAllLogs();
    void showStats();

private:
    void setupUI();
    void refreshTable();
    User m_user;
    Database m_db;
    QTableView* m_tableView;
    QSqlTableModel* m_model;
};

#endif // MAINWINDOW_H
