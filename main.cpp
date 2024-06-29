#include "mainwindow.h"
#include <json/json.h>
#include <string>
#include <iostream>
#include <QApplication>

// it is a test to verify if jsoncpp works.
// json is supposed to be used in messages.
void testjson()
{
    Json::Value root;
    root["key"] = "value";
}

int main(int argc, char *argv[])
{
    testjson();
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(800, 350);
    w.show();
    return a.exec();
}
