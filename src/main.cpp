#include <QApplication>
#include "AuthDialog.h"
#include "MainWindow.h"
#include "Database.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Система управления угрозами ИБ");

    // Инициализация базы данных в подпапке data/
    Database db;
    if (!db.open("data/threats_manager.db")) {
        qCritical() << "Ошибка: не удалось открыть базу данных!";
        return 1;
    }

    AuthDialog auth;
    if (auth.exec() == QDialog::Accepted) {
        // Передаём пользователя в главное окно
        MainWindow* w = new MainWindow(auth.getCurrentUser());
        w->show();
        return app.exec();
    }

    return 0;
}
