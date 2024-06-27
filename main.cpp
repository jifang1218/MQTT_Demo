#include "mainwindow.h"
#include <json/json.h>
#include <string>
#include <iostream>

#include <QApplication>
void testjson()
{
    // 示例 JSON 字符串
    std::string rawJson = R"({
            "name": "John Doe",
            "age": 30,
            "isStudent": false,
            "address": {
                "street": "123 Main St",
                "city": "Anytown",
                "zip": "12345"
            },
            "phoneNumbers": ["123-456-7890", "987-654-3210"]
        })";
    
    // 解析 JSON
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader* reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    
    if (reader->parse(rawJson.c_str(), rawJson.c_str() + rawJson.size(), &root, &errors)) {
        // 访问 JSON 数据
        std::string name = root["name"].asString();
        int age = root["age"].asInt();
        bool isStudent = root["isStudent"].asBool();
        std::string street = root["address"]["street"].asString();
        std::string city = root["address"]["city"].asString();
        std::string zip = root["address"]["zip"].asString();
        Json::Value phoneNumbers = root["phoneNumbers"];
        
        std::cout << "Name: " << name << std::endl;
        std::cout << "Age: " << age << std::endl;
        std::cout << "Is Student: " << (isStudent ? "true" : "false") << std::endl;
        std::cout << "Address: " << street << ", " << city << ", " << zip << std::endl;
        std::cout << "Phone Numbers: ";
        for (const auto& number : phoneNumbers) {
            std::cout << number.asString() << " ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Failed to parse the JSON, errors: " << errors << std::endl;
    }
    
    delete reader;
    
    // 生成 JSON
    Json::Value newRoot;
    newRoot["title"] = "JsonCpp Example";
    newRoot["year"] = 2024;
    newRoot["isExample"] = true;
    newRoot["data"]["key1"] = "value1";
    newRoot["data"]["key2"] = "value2";
    
    Json::StreamWriterBuilder writerBuilder;
    std::string newJson = Json::writeString(writerBuilder, newRoot);
    std::cout << "Generated JSON: " << newJson << std::endl;
}
int main(int argc, char *argv[])
{
    testjson();
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(800, 600);
    w.show();
    return a.exec();
}
