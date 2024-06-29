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
#include <QCheckBox>
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

#pragma mark - UI Part
ClientWindow::ClientWindow(const MQTTLoginInfo &loginInfo)
{
    _client = new MQTTClient(loginInfo);
    setupCallbacks();
    
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

void ClientWindow::setupCallbacks()
{
    connect(this, &ClientWindow::jump2UISignal, this, &ClientWindow::jump2UISlot);
    _client->onConnectHandler = [](int errCode, const MQTTClient *cli){};
    _client->onConnectHandler = std::bind(&ClientWindow::_onConnectCompleted, this,
                                          std::placeholders::_1, std::placeholders::_2);
    _client->onDisconnectHandler = std::bind(&ClientWindow::_onDisconnectCompleted, this,
                                             std::placeholders::_1,
                                             std::placeholders::_2);
    _client->onPublishMessageHandler = std::bind(&ClientWindow::_onPublishMessageCompleted, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2,
                                                 std::placeholders::_3);
    _client->onSubscribeTopicHandler = std::bind(&ClientWindow::_onSubscribeTopicCompleted, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2);
    _client->onUnsubscribeTopicHandler = std::bind(&ClientWindow::_onUnsubscribeTopicCompleted, this,
                                                   std::placeholders::_1,
                                                   std::placeholders::_2);
    _client->onMessageReceivedHandler = std::bind(&ClientWindow::_onMessageReceived, this,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2);
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
    lineEdit->setReadOnly(true);
    lineEdit->setText(QString::fromStdString(_client->GetServer()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    label = new QLabel("Client ID: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setReadOnly(true);
    lineEdit->setText(QString::fromStdString(_client->GetClientId()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    label = new QLabel("Display Name: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setText(QString::fromStdString(_client->GetDisplayName()));
    lineEdit->setReadOnly(true);
    gridLayout->addWidget(lineEdit, row++, 1);
    
    QVBoxLayout *vLayout = new QVBoxLayout();
    label = new QLabel("Description");
    vLayout->addWidget(label);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setText(QString::fromStdString(_client->GetDescription()));
    textEdit->setReadOnly(true);
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
    lwSubscribedTopics = new QListWidget();
    vector<string> topics = _client->GetSubTopics();
    sort(topics.begin(), topics.end());
    for (const string &topic : topics) {
        lwSubscribedTopics->addItem(QString::fromStdString(topic));
    }
    gridLayout->addWidget(lwSubscribedTopics, 0, 0);
    QPushButton *button = new QPushButton("Unsubscribe");
    connect(button, &QPushButton::clicked, this, [this]()->void {
        int row = lwSubscribedTopics->currentRow();
        if (row >= 0) {
            vector<string> topics = _client->GetSubTopics();
            string topic = topics[row];
            _client->UnsubTopic(topic);
        }
    });
    button->setEnabled(lwSubscribedTopics->currentRow() >= 0);
    connect(lwSubscribedTopics, &QListWidget::itemSelectionChanged, [button, this]()->void{
        QList<QListWidgetItem *> selectedItems = lwSubscribedTopics->selectedItems();
        if (selectedItems.size() > 0 &&
            lwSubscribedTopics->currentRow() >= 0) {
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
    lwPublishedTopics = new QListWidget();
    topics = _client->GetPubTopics();
    sort(topics.begin(), topics.end());
    for (const string &topic : topics) {
        lwPublishedTopics->addItem(QString::fromStdString(topic));
    }
    connect(lwPublishedTopics, &QListWidget::itemSelectionChanged, this, &ClientWindow::onPubTopicSelectionChanged);
    gridLayout->addWidget(lwPublishedTopics, 0, 0);
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
    connect(button, &QPushButton::clicked, this, [lineEdit, this]()->void {
        string topic = lineEdit->text().toStdString();
        this->_client->SubTopic(topic);
    });
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
    edtPubTopic = new QLineEdit();
    edtPubTopic->setPlaceholderText("Topic");
    hLayout->addWidget(edtPubTopic);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout();
    label = new QLabel("Message: ");
    hLayout->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit();
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

QLayout *ClientWindow::construct_rest()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();
    btnConnect = new QPushButton("Connect");
    btnConnect->setEnabled(!_client->IsConnected());
    connect(btnConnect, &QPushButton::clicked, this, &ClientWindow::onConnectButtonClicked);
    vLayout->addWidget(btnConnect);
    btnDisconnect = new QPushButton("Disconnect");
    connect(btnDisconnect, &QPushButton::clicked, this, &ClientWindow::onDisconnectButtonClicked);
    btnDisconnect->setEnabled(_client->IsConnected());
    vLayout->addWidget(btnDisconnect);
    QCheckBox *checkBox = new QCheckBox("Auto-Reconnect");
    checkBox->setChecked(false);
    connect(checkBox, &QCheckBox::stateChanged, [checkBox, this](){
        this->_client->SetAutoReconnectEnabled(checkBox->isChecked());
    });
    vLayout->addWidget(checkBox);
    hLayout->addLayout(vLayout);
    
    vLayout = new QVBoxLayout();
    QListWidget *listWidget = new QListWidget();
#if TEST
    listWidget->addItem("recv: topic:aaa, msg:bbb");
    listWidget->addItem("recv: topic:aaa, msg:bbb");
    listWidget->addItem("recv: topic:aaa, msg:bbb");
    listWidget->addItem("recv: topic:aaa, msg:bbb");
    listWidget->addItem("recv: topic:aaa, msg:bbb");
#endif
    vLayout->addWidget(listWidget);
    lblStatus = new QLabel();
    if (_client->IsConnected()) {
        lblStatus->setText("Connected!");
    } else {
        lblStatus->setText("Disconnected!");
    }
    vLayout->addWidget(lblStatus);
    hLayout->addLayout(vLayout);
    
    return hLayout;
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
    
    layout = construct_rest();
    mainLayout->addLayout(layout, 2, 0, 1, 2);
}

void ClientWindow::onPubTopicSelectionChanged()
{
    QListWidget *listWidget = static_cast<QListWidget *>(QObject::sender());
    if (listWidget->currentRow() >= 0) {
        edtPubTopic->setText(listWidget->currentItem()->text());
    }
}

void ClientWindow::onConnectButtonClicked()
{
    if (!_client->IsConnected()) {
        _client->Connect();
    }
}
void ClientWindow::onDisconnectButtonClicked()
{
    if (_client->IsConnected()) {
        _client->Disconnect();
    }
}

#pragma mark - MQTTClient Callbacks
void ClientWindow::jump2UISlot(int type, int arg1, const QString &arg2, const QString &arg3, const void *client)
{
    switch (type) {
        case (int)CallbackType::Connect: {
            onConnectCompleted(arg1, (const Fang::MQTTClient *)client);
        } break;
        case (int)CallbackType::Disconnect: {
            onDisconnectCompleted(arg1, (const Fang::MQTTClient *)client);
        } break;
        case (int)CallbackType::Publish: {
            onPublishMessageCompleted(arg1, arg2.toStdString(), arg3.toStdString());
        } break;
        case (int)CallbackType::Subscribe: {
            onSubscribeTopicCompleted(arg1, arg2.toStdString());
        } break;
        case (int)CallbackType::Unsubscribe: {
            onUnsubscribeTopicCompleted(arg1, arg2.toStdString());
        } break;
        case (int)CallbackType::MessageReceived: {
            onMessageReceived(arg2.toStdString(), arg3.toStdString());
        } break;
        default: {
        } break;
    }
}
    
void ClientWindow::onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &data)
{
    bool success = errCode == 0;
    if (success) {
        lwPublishedTopics->clear();
        vector<string> topics = _client->GetPubTopics();
        sort(topics.begin(), topics.end());
        for (const string &topic : topics) {
            lwPublishedTopics->addItem(QString::fromStdString(topic));
        }
        lblStatus->setText("Publish Topic: " + QString::fromStdString(topic) + "with message: " + QString::fromStdString(data) + " -- OK!");
    } else {
        lblStatus->setText("Publish Topic: " + QString::fromStdString(topic) + "with message: " + QString::fromStdString(data) + " -- Failed!");
    }
}

void ClientWindow::onSubscribeTopicCompleted(int errCode, const std::string &topic)
{
    bool success = errCode == 0;
    if (success) {
        lwSubscribedTopics->clear();
        vector<string> topics = _client->GetSubTopics();
        sort(topics.begin(), topics.end());
        for (const string &topic : topics) {
            lwSubscribedTopics->addItem(QString::fromStdString(topic));
        }
        lblStatus->setText("Subscribe Topic: " + QString::fromStdString(topic) + " -- OK!");
    } else {
        lblStatus->setText("Subscribe Topic: " + QString::fromStdString(topic) + " -- Failed!");
    }
}

void ClientWindow::onUnsubscribeTopicCompleted(int errCode, const std::string &topic)
{
    bool success = errCode == 0;
    if (success) {
        lwSubscribedTopics->clear();
        vector<string> topics = _client->GetSubTopics();
        sort(topics.begin(), topics.end());
        for (const string &topic : topics) {
            lwSubscribedTopics->addItem(QString::fromStdString(topic));
        }
        lblStatus->setText("Unsubscribe Topic: " + QString::fromStdString(topic) + " -- OK!");
    } else {
        lblStatus->setText("Unsubscribe Topic: " + QString::fromStdString(topic) + " -- Failed!");
    }
}

void ClientWindow::onConnectCompleted(int errCode, const MQTTClient *self)
{
    bool success = errCode == 0;
    this->btnConnect->setEnabled(!success);
    this->btnDisconnect->setEnabled(success);
    this->lblStatus->setText(success?"Connected!":"Connect Failure, errCode: " + QString::number(errCode));
}

void ClientWindow::onDisconnectCompleted(int errCode, const MQTTClient *self)
{
    bool success = errCode == 0;
    this->btnConnect->setEnabled(success);
    this->btnDisconnect->setEnabled(!success);
    this->lblStatus->setText(success?"Disconnected!":"Disconnect Failure, errCode: " + QString::number(errCode));
}

void ClientWindow::onMessageReceived(const std::string &topic, const std::string &message)
{
}


void ClientWindow::_onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &data)
{
    emit jump2UISignal((int)CallbackType::Publish, errCode, QString::fromStdString(topic), QString::fromStdString(data), nullptr);
}

void ClientWindow::_onSubscribeTopicCompleted(int errCode, const std::string &topic)
{
    emit jump2UISignal((int)CallbackType::Subscribe, errCode, QString::fromStdString(topic), "", nullptr);
}

void ClientWindow::_onUnsubscribeTopicCompleted(int errCode, const std::string &topic)
{
    emit jump2UISignal((int)CallbackType::Unsubscribe, errCode, QString::fromStdString(topic), "", nullptr);
}

void ClientWindow::_onConnectCompleted(int errCode, const MQTTClient *self)
{
    emit jump2UISignal((int)CallbackType::Connect, errCode, "", "", self);
}

void ClientWindow::_onDisconnectCompleted(int errCode, const MQTTClient *self)
{
    emit jump2UISignal((int)ClientWindow::CallbackType::Disconnect, errCode, "", "", self);
}

void ClientWindow::_onMessageReceived(const std::string &topic, const std::string &message)
{
    emit jump2UISignal(static_cast<int>(CallbackType::MessageReceived), 0, QString::fromStdString(topic), QString::fromStdString(message), nullptr);
}
