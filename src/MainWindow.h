#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "User.h"
#include "Database.h"

class QTableView;

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
};

#endif 
