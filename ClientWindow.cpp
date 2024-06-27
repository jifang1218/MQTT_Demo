//
//  ClientWindow.cpp
//  mqtttest0
//
//  Created by jifang on 2024-06-25.
//

#include "ClientWindow.hpp"
#include "MQTTClient.hpp"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QListWidget>
#include <QComboBox>
#include <QList>
#include <algorithm>
#include <QDebug>

using namespace Fang;
using namespace std;

ClientWindow::ClientWindow(const MQTTLoginInfo &loginInfo)
{
    _client = new MQTTClient(loginInfo);
    construct();
    QString title;
    if (_client->GetDisplayName().length()>0) {
        title += QString::fromStdString(_client->GetDisplayName());
    } else {
        title += "No Name";
    }
    title += " (Client ID: ";
    title += QString::fromStdString(_client->GetClientId());
    title += ")";
    setWindowTitle(title);
}

ClientWindow::~ClientWindow()
{
    delete _client;
}

std::string ClientWindow::GetClientId() const {
    return _client->GetClientId();
}

QWidget *ClientWindow::construct0_0()
{
    int row = 0;
    QGroupBox *groupBox = new QGroupBox("Info");
    QGridLayout *gridLayout = new QGridLayout();
    groupBox->setLayout(gridLayout);
    QLabel *label = new QLabel("Server: ");
    gridLayout->addWidget(label, row, 0);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setEnabled(false);
    lineEdit->setText(QString::fromStdString(_client->GetServer()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    label = new QLabel("Client ID: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setEnabled(false);
    lineEdit->setText(QString::fromStdString(_client->GetClientId()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    label = new QLabel("Display Name: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setText(QString::fromStdString(_client->GetDisplayName()));
    lineEdit->setEnabled(false);
    gridLayout->addWidget(lineEdit, row++, 1);
    
    QVBoxLayout *vLayout = new QVBoxLayout();
    label = new QLabel("Description");
    vLayout->addWidget(label);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setText(QString::fromStdString(_client->GetDescription()));
    textEdit->setEnabled(false);
    vLayout->addWidget(textEdit);
    vLayout->addStretch();
    gridLayout->addLayout(vLayout, row, 0, 1, 2);
    
    return groupBox;
}

QWidget *ClientWindow::construct0_1()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    QGroupBox *groupBox = new QGroupBox("Subscribed Topics");
    QGridLayout *gridLayout = new QGridLayout();
    QListWidget *listWidget = new QListWidget();
    vector<string> topics = _client->GetSubTopics();
    sort(topics.begin(), topics.end());

    vector<string> testData;
    testData.push_back("ddd");
    testData.push_back("add");
    testData.push_back("abd");
    testData.push_back("ded");
    std::sort(testData.begin(), testData.end());
    //for (const string &topic : topics) {
    for (const string &topic : testData) {
        listWidget->addItem(QString::fromStdString(topic));
    }
    gridLayout->addWidget(listWidget, 0, 0);
    QPushButton *button = new QPushButton("Unsubscribe");
#if TEST
    if (testData.size() == 0) {
#else
    if (topics.size() == 0) {
#endif
        button->setEnabled(false);
    }
    connect(listWidget, &QListWidget::itemSelectionChanged, [button, listWidget]()->void{
        QList<QListWidgetItem *> selectedItems = listWidget->selectedItems();
        if (selectedItems.size() > 0) {
            button->setEnabled(true);
        } else {
            button->setEnabled(false);
        }
    });
    gridLayout->addWidget(button, 0, 1);
    groupBox->setLayout(gridLayout);
    vLayout->addWidget(groupBox);
    
    groupBox = new QGroupBox("Recent Published Topics");
    gridLayout = new QGridLayout();
    listWidget = new QListWidget();
    topics = _client->GetPubTopics();
    sort(topics.begin(), topics.end());
    
    testData.clear();
    testData.push_back("ddd");
    testData.push_back("add");
    testData.push_back("abd");
    testData.push_back("ded");
    std::sort(testData.begin(), testData.end());
    //for (const string &topic : topics) {
    for (const string &topic : testData) {
        listWidget->addItem(QString::fromStdString(topic));
    }
    gridLayout->addWidget(listWidget, 0, 0);
    groupBox->setLayout(gridLayout);
    vLayout->addWidget(groupBox);
    
    return reinterpret_cast<QWidget *>(vLayout);
}

QWidget *ClientWindow::construct1_0()
{
    QGroupBox *groupBox = new QGroupBox("Subscribe");
    QVBoxLayout *vLayout = new QVBoxLayout();
    groupBox->setLayout(vLayout);
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Topic: ");
    hLayout->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Topic");
    hLayout->addWidget(lineEdit);
    vLayout->addLayout(hLayout);
    QPushButton *button = new QPushButton("Subscribe");
    vLayout->addWidget(button);
    
    return groupBox;
}

QWidget *ClientWindow::construct1_1()
{
    QGroupBox *groupBox = new QGroupBox("Publish");
    QVBoxLayout *vLayout = new QVBoxLayout();
    groupBox->setLayout(vLayout);
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Topic: ");
    hLayout->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Topic");
    hLayout->addWidget(lineEdit);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout();
    label = new QLabel("Message: ");
    hLayout->addWidget(label);
    lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Message");
    hLayout->addWidget(lineEdit);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout();
    label = new QLabel("QoS: ");
    hLayout->addWidget(label);
    QComboBox *cmbQos = new QComboBox();
    cmbQos->addItem("0");
    cmbQos->addItem("1");
    cmbQos->addItem("2");
    cmbQos->setCurrentIndex(1);
    hLayout->addWidget(cmbQos);
    QPushButton *button = new QPushButton("Publish");
    hLayout->addWidget(button);
    vLayout->addLayout(hLayout);
    
    return groupBox;
}

void ClientWindow::construct()
{
    QGridLayout *mainLayout = new QGridLayout;
    setLayout(mainLayout);
    
    // client info
    QWidget *w0_0 = construct0_0();
    mainLayout->addWidget(w0_0, 0, 0);
    
    // subscribe topic
    QWidget *w1_0 = construct1_0();
    mainLayout->addWidget(w1_0, 1, 0);
    
    // publish message
    QWidget *w1_1 = construct1_1();
    mainLayout->addWidget(w1_1, 1, 1);
    
    // Subscribed Topics
    QWidget *w0_1 = construct0_1();
    QLayout *layout = reinterpret_cast<QLayout *>(w0_1);
    mainLayout->addLayout(layout, 0, 1);
}
