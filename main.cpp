#include "sqlite/sqlite3.h"

#include "config.h"
#include "mainwindow.h"

void regexp(sqlite3_context* context, int /*argc*/, sqlite3_value* argv[])
{
    QRegExp rx(QRegExp(reinterpret_cast<const char*>(sqlite3_value_text(argv[0]))));
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    sqlite3_result_int(context, QString(reinterpret_cast<const char*>(sqlite3_value_text(argv[1]))).contains(rx) ? 1 : 0);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont(":/rc/siddhanta-calcutta.ttf");

    Config config;

    QString locale = config.langs.value(config.lang);

    QTranslator trans1;
    trans1.load(":/rc/sandic_" + locale);
    app.installTranslator(&trans1);

    QTranslator trans2;
    trans2.load(":/rc/qt_" + locale);
    app.installTranslator(&trans2);

    QString dbName = "sandic.db";

    QStringList argvList = QCoreApplication::arguments();

    for (int i = 0; i < argvList.count(); i++)
        if (argvList[i] == "db") {
            dbName = argvList[i + 1];
            break;
        }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);

    if (!QFile::exists(dbName) || !db.open()) {
        QMessageBox::critical(0, QObject::tr("Database error"), QObject::tr("Can't open database file: %1").arg(dbName));
        exit(-1);
    }
    // 1. sqlite3_create_function без sqlite3_initialize вызывает ошибку
    // 2. sqlite3_create_function на работает в собранной 64битной версии (mingw73_64)
    sqlite3_initialize();
    sqlite3_create_function(*static_cast<sqlite3**>(db.driver()->handle().data()), "regexp", 2, SQLITE_UTF8, NULL, &regexp, NULL, NULL);

    QSqlQuery query("PRAGMA cache_size = -150000", db); // 153.600.000b

    QSplashScreen splash(QPixmap(QString(":/rc/splash_%1.png").arg(locale)));
    splash.show();

    MainWindow window(&config, &db);
    window.show();
    
    splash.finish(&window);

    return app.exec();
}