//
//  ClientWindow.cpp
//  mqtttest0
//
//  Created by jifang on 2024-06-25.
//

#include "ClientWindow.hpp"
#include "MQTTClient.hpp"
#include <QMessageBox>
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
#include <QDateTime>

using namespace Fang;
using namespace std;

#pragma mark - UI Part
ClientWindow::ClientWindow(const MQTTLoginInfo &loginInfo)
{
    _client = new MQTTClient(loginInfo);
    setupCallbacks();
    
    construct();
    /*
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
     */
}

ClientWindow::~ClientWindow()
{
    delete _client;
}

void ClientWindow::closeEvent(QCloseEvent *event) {
    emit widgetClosed(); // Emit close signal
    QWidget::closeEvent(event);
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
    // Server: ______
    QLabel *label = new QLabel("Server: ");
    gridLayout->addWidget(label, row, 0);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setReadOnly(true);
    lineEdit->setText(QString::fromStdString(_client->GetServer()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    // Client ID: ______
    label = new QLabel("Client ID: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setReadOnly(true);
    lineEdit->setText(QString::fromStdString(_client->GetClientId()));
    gridLayout->addWidget(lineEdit, row++, 1);
    
    // Display Name: ______
    label = new QLabel("Display Name: ");
    gridLayout->addWidget(label, row, 0);
    lineEdit = new QLineEdit();
    lineEdit->setText(QString::fromStdString(_client->GetDisplayName()));
    lineEdit->setReadOnly(true);
    gridLayout->addWidget(lineEdit, row++, 1);
    
    // Description: ______
    QVBoxLayout *vLayout = new QVBoxLayout();
    label = new QLabel("Description");
    vLayout->addWidget(label);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setText(QString::fromStdString(_client->GetDescription()));
    textEdit->setReadOnly(true);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vLayout->addWidget(textEdit);
    //vLayout->addStretch();
    gridLayout->addLayout(vLayout, row, 0, 1, 2);
    
    return groupBox;
}

QWidget *ClientWindow::construct0_1()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    
    // Subscribed Topics: ______
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
        if (!_client->IsConnected()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Cannot Process.");
            msgBox.setText("Please connect to server first.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }
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
    
    // Recent Published Topics: ______
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
    
    // Topic: ______
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Topic: ");
    hLayout->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Topic");
    hLayout->addWidget(lineEdit);
    
    // QoS: []
    label = new QLabel("QoS: ");
    hLayout->addWidget(label);
    QComboBox *cmbSubQos = new QComboBox();
    cmbSubQos->addItem("0");
    cmbSubQos->addItem("1");
    cmbSubQos->addItem("2");
    cmbSubQos->setCurrentIndex(1);
    hLayout->addWidget(cmbSubQos);
    vLayout->addLayout(hLayout);
    btnSubscribe = new QPushButton("Subscribe");
    btnSubscribe->setEnabled(_client->IsConnected());
    connect(btnSubscribe, &QPushButton::clicked, this, [cmbSubQos, lineEdit, this]()->void {
        if (!_client->IsConnected()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Cannot Process.");
            msgBox.setText("Please connect to server first.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }
        string topic = lineEdit->text().toStdString();
        this->_client->SubTopic(topic, cmbSubQos->currentIndex());
    });
    vLayout->addWidget(btnSubscribe);
    
    return groupBox;
}

QWidget *ClientWindow::construct1_1()
{
    QGroupBox *groupBox = new QGroupBox("Publish");
    QVBoxLayout *vLayout = new QVBoxLayout();
    groupBox->setLayout(vLayout);
    
    // Topic: ______
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Topic: ");
    hLayout->addWidget(label);
    edtPubTopic = new QLineEdit();
    edtPubTopic->setPlaceholderText("Topic");
    hLayout->addWidget(edtPubTopic);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout();
    
    // Message: ______
    label = new QLabel("Message: ");
    hLayout->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Message");
    hLayout->addWidget(lineEdit);
    vLayout->addLayout(hLayout);
    
    // QoS: []
    hLayout = new QHBoxLayout();
    label = new QLabel("QoS: ");
    hLayout->addWidget(label);
    QComboBox *cmbQos = new QComboBox();
    cmbQos->addItem("0");
    cmbQos->addItem("1");
    cmbQos->addItem("2");
    cmbQos->setCurrentIndex(1);
    hLayout->addWidget(cmbQos);
    btnPublish = new QPushButton("Publish");
    btnPublish->setEnabled(_client->IsConnected());
    connect(btnPublish, &QPushButton::clicked, this, [this, lineEdit, cmbQos]()->void {
        if (!_client->IsConnected()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Cannot Process.");
            msgBox.setText("Please connect to server first.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }
        string topic = edtPubTopic->text().toStdString();
        string message = lineEdit->text().toStdString();
        int qos = cmbQos->currentIndex();
        _client->PubMessageForTopic(topic, message, qos);
    });
    hLayout->addWidget(btnPublish);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);
    
    return groupBox;
}

QLayout *ClientWindow::construct_rest()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();
    
    // buttons: Connect, Disconnect, Auto-Reconnect
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
    vLayout->addStretch();
    hLayout->addLayout(vLayout);
    
    vLayout = new QVBoxLayout();
    lwReceivedMessages = new QListWidget();
    vLayout->addWidget(lwReceivedMessages);
    
    // Status: ______
    lblStatus = new QLabel();
    if (_client->IsConnected()) {
        lblStatus->setText("Status: Connected!");
    } else {
        lblStatus->setText("Status: Disconnected!");
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
        lblStatus->setText("Status: Publish Topic: "
                           + QString::fromStdString(topic)
                           + " with message: "
                           + QString::fromStdString(data)
                           + " -- OK!");
    } else {
        lblStatus->setText("Status: Publish Topic: "
                           + QString::fromStdString(topic)
                           + " with message: "
                           + QString::fromStdString(data)
                           + " -- Failed, errCode: " + QString::number(errCode));
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
        lblStatus->setText("Status: Subscribe Topic: "
                           + QString::fromStdString(topic)
                           + " -- OK!");
    } else {
        lblStatus->setText("Status: Subscribe Topic: "
                           + QString::fromStdString(topic)
                           + " -- Failed, errCode: " + QString::number(errCode));
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
        lblStatus->setText("Status: Unsubscribe Topic: "
                           + QString::fromStdString(topic)
                           + " -- OK!");
    } else {
        lblStatus->setText("Status: Unsubscribe Topic: "
                           + QString::fromStdString(topic)
                           + " -- Failed, errCode: " + QString::number(errCode));
    }
}

void ClientWindow::onConnectCompleted(int errCode, const MQTTClient *self)
{
    bool success = errCode == 0;
    this->btnConnect->setEnabled(!success);
    this->btnDisconnect->setEnabled(success);
    if (success) {
        lblStatus->setText("Status: Connected!");
    } else {
        lblStatus->setText(QString("Status: Connect Failure, errCode: ") + QString::number(errCode));
    }
    
    btnSubscribe->setEnabled(success);
    btnPublish->setEnabled(success);
}

void ClientWindow::onDisconnectCompleted(int errCode, const MQTTClient *self)
{
    bool success = errCode == 0;
    this->btnConnect->setEnabled(success);
    this->btnDisconnect->setEnabled(!success);
    if (success) {
        lblStatus->setText("Status: Disconnected!");
    } else {
        lblStatus->setText(QString("Status: Disconnect Failure, errCode: ") + QString::number(errCode));
    }
    btnSubscribe->setEnabled(!success);
    btnPublish->setEnabled(!success);
}

void ClientWindow::onMessageReceived(const std::string &topic, const std::string &message)
{
    QString str = "";
    str += QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    str += " | recv: topic: " + QString::fromStdString(topic) + " | message: " + QString::fromStdString(message);
    lwReceivedMessages->insertItem(0, str);
}

// the following methods are used for switching to UI thread by emitting signal jump2UISignal.
// then the slot jump2UISlot will be running in UI thread.
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
