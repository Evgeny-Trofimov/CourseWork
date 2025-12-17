#include <QApplication>
#include "AuthDialog.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Система управления угрозами ИБ");

    AuthDialog auth;
    if (auth.exec() == QDialog::Accepted) {
        return app.exec();
    }
    return 0;
}
