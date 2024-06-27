#include "mainwindow.h"
#include "Utils.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QIntValidator>
#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>
#include "ClientWindow.hpp"
#include "ServerWindow.hpp"
#include "MQTTLoginInfo.hpp"

using namespace std;
using namespace Fang;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construct();
}

MainWindow::~MainWindow() {
    for (auto &item : _clients) {
        delete item.second;
    }
    _clients.clear();
    
    delete pServerWindow;
}

QWidget *MainWindow::createServerBox() {
    QGroupBox *groupBox = new QGroupBox("Server");
    
    QGridLayout *gridLayout = new QGridLayout(groupBox);
    QVBoxLayout *vLayout = new QVBoxLayout();
    serverButtonGroup = new QButtonGroup(groupBox);
    QRadioButton *radioButton = new QRadioButton("Start New Server");
    serverButtonGroup->addButton(radioButton, 0);
    radioButton->setChecked(false);
    vLayout->addWidget(radioButton);
    radioButton = new QRadioButton("Connect to Server");
    serverButtonGroup->addButton(radioButton, 1);
    radioButton->setChecked(true);
    vLayout->addWidget(radioButton);
    gridLayout->addLayout(vLayout, 0, 0);
    
    edtConnectToServer = new QLineEdit(groupBox);
    edtConnectToServer->setPlaceholderText("Server IP or address:");
    gridLayout->addWidget(edtConnectToServer, 0, 1);
    
    edtConnectToServerPort = new QLineEdit();
    QIntValidator *intValidator = new QIntValidator();
    edtConnectToServerPort->setValidator(intValidator);
    edtConnectToServerPort->setPlaceholderText("Port");
    gridLayout->addWidget(edtConnectToServerPort, 0, 2);
    connect(serverButtonGroup, &QButtonGroup::idToggled, this, &MainWindow::onServerCreationButtonToggled);
    
    QPushButton *button = new QPushButton("Go!");
    connect(button, &QPushButton::clicked, this, &MainWindow::onGoButtonClicked);
    gridLayout->addWidget(button, 2, 1);
    groupBox->setLayout(gridLayout);
    
    return groupBox;
}

QWidget *MainWindow::createClientBox() {
    QGroupBox *groupBox = new QGroupBox("Client");
    
    // col 0
    int row = 0, col = 0;
    QGridLayout *gridLayout = new QGridLayout(groupBox);
    edtServer = new QLineEdit();
    edtServer->setPlaceholderText("Server IP or address");
    gridLayout->addWidget(edtServer, row++, col);
    
    edtClientId = new QLineEdit();
    edtClientId->setPlaceholderText("Client ID");
    gridLayout->addWidget(edtClientId, row++, col);
    
    QVBoxLayout *vLayout = new QVBoxLayout();
    edtUsername = new QLineEdit();
    edtUsername->setPlaceholderText("username");
    vLayout->addWidget(edtUsername);
    
    edtPasswd = new QLineEdit();
    edtPasswd->setEchoMode(QLineEdit::Password);
    edtPasswd->setPlaceholderText("password");
    vLayout->addWidget(edtPasswd);
    
    cbAnonymousLogin = new QCheckBox("Anonymous Login");
    cbAnonymousLogin->setChecked(false);
    connect(cbAnonymousLogin, &QCheckBox::stateChanged, this, &MainWindow::onAnonymousLoginToggled);
    vLayout->addWidget(cbAnonymousLogin);
    gridLayout->addLayout(vLayout, row++, col);
    
    // col 1
    row = 0; ++col;
    edtPort = new QLineEdit();
    QIntValidator *intValidator = new QIntValidator();
    edtPort->setValidator(intValidator);
    edtPort->setPlaceholderText("Port");
    gridLayout->addWidget(edtPort, row++, col);
    
    QPushButton *button = new QPushButton("Generate ID");
    connect(button, &QPushButton::clicked, this, &MainWindow::onGenerateButtonClicked);
    gridLayout->addWidget(button, row++, col);
    
#if TEST
    button = new QPushButton("Dummy Client");
    connect(button, &QPushButton::clicked, this, &MainWindow::onDummyClientButtonClicked);
    gridLayout->addWidget(button, row++, col);
#endif
    
    button = new QPushButton("Run Client");
    connect(button, &QPushButton::clicked, this, &MainWindow::onRunClientButtonClicked);
    gridLayout->addWidget(button, row++, col);
    
    // col 2
    int rowspan = row - 1;
    row = 0; ++col;
    edtDisplayName = new QLineEdit();
    edtDisplayName->setPlaceholderText("Display Name");
    gridLayout->addWidget(edtDisplayName, row++, col);
    teDescription = new QTextEdit();
    teDescription->setPlaceholderText("Client Description");
    gridLayout->addWidget(teDescription, row++, col, rowspan, 1);
    
    groupBox->setLayout(gridLayout);
    
    return groupBox;
}

void MainWindow::construct() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    central->setLayout(mainLayout);
    
    // server
    QGroupBox *gbServer = static_cast<QGroupBox *>(createServerBox());
    mainLayout->addWidget(gbServer);
    
    // client
    QGroupBox *gbClient = static_cast<QGroupBox *>(createClientBox());
    mainLayout->addWidget(gbClient);

    this->setCentralWidget(central);
}

void MainWindow::onGoButtonClicked()
{
    qDebug() << "go button clicked." << Qt::endl;
    qDebug() << serverButtonGroup->checkedId() << Qt::endl;
}

void MainWindow::onGenerateButtonClicked()
{
    const int len = 50;
    string clientId = Utils::generateString(len);
    while(_clients.find(clientId) != _clients.end()) {
        clientId = Utils::generateString(len);
    }
    
    edtClientId->setText(QString::fromStdString(clientId));
}

void MainWindow::onRunClientButtonClicked()
{
    if (validateClientInput()) {
        string server = "tcp://"
                        + edtServer->text().toStdString()
                        + ":"
                        + edtPort->text().toStdString();
        string clientId = edtClientId->text().toStdString();
        if (_clients.find(clientId) != _clients.end()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Cannot create client.");
            msgBox.setText("Client ID already exists, please use a new client ID.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }
        string username = "";
        string password = "";
        if (!cbAnonymousLogin->isChecked()) {
            username = edtUsername->text().toStdString();
            password = edtPasswd->text().toStdString();
        }
        
        MQTTLoginInfo login;
        login.host = server;
        login.clientId = clientId;
        login.username = username;
        login.password = password;
        login.displayName = edtDisplayName->text().toStdString();
        login.description = teDescription->toPlainText().toStdString();
        
        ClientWindow *pClient = new ClientWindow(login);
        _clients[clientId] = pClient;
        pClient->resize(800, 450);
        pClient->show();
    }
}

void MainWindow::onServerCreationButtonToggled(int id, bool checked)
{
    if (checked) {
        switch (id) {
            case 0: { // start new server
                edtConnectToServer->setText("localhost");
                edtConnectToServer->setEnabled(false);
                edtConnectToServerPort->setText("1883");
                edtConnectToServerPort->setEnabled(false);
            } break;
            case 1: { // connect to
                edtConnectToServer->setText("");
                edtConnectToServer->setEnabled(true);
                edtConnectToServerPort->setText("");
                edtConnectToServerPort->setEnabled(true);
            } break;
            default: {
            } break;
        }
    }
}

bool MainWindow::validateClientInput() {
    bool ret = false;
    do {
        if (edtServer->text().trimmed().length() <= 0) {
            break;
        }
        if (edtPort->text().trimmed().length() <= 0) {
            break;
        }
        if (edtClientId->text().trimmed().length() <= 0) {
            break;
        }
        // anonymous login.
        if (!cbAnonymousLogin->isChecked()) {
            if (edtUsername->text().trimmed().length() <= 0) {
                break;
            }
            if (edtPasswd->text().trimmed().length() <= 0) {
                //break; // allow empty password login.
            }
        }
        ret = true;
    } while (0);
    
    return ret;
}

#if TEST
void MainWindow::onDummyClientButtonClicked()
{
    edtServer->setText("localhost");
    edtPort->setText("1883");
    // set client id
    onGenerateButtonClicked();
    edtUsername->setText("jifang");
    edtPasswd->setText("jifang");
    cbAnonymousLogin->setChecked(false);
    edtDisplayName->setText("Dummy Client");
    teDescription->setText("this is a dummy client used to test MQTT.\nYou can run the client to connect to the local server.");
}
#endif

void MainWindow::onAnonymousLoginToggled(int state)
{
    bool isEnabled = true;
    if (state == Qt::Checked) { // unchecked.
        isEnabled = false;
    }
    edtUsername->setEnabled(isEnabled);
    edtPasswd->setEnabled(isEnabled);
}
