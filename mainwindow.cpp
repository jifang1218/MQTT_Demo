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
    for (auto &item : _clientWindows) {
        delete item.second;
    }
    _clientWindows.clear();
    
    delete pServerWindow;
}

QWidget *MainWindow::createServerBox() {
    QGroupBox *groupBox = new QGroupBox("Server");
    
    // Connect to Server: ______
    QHBoxLayout *mainLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Connect to Server: ");
    mainLayout->addWidget(label);
    edtConnectToServer = new QLineEdit(groupBox);
    edtConnectToServer->setPlaceholderText("Server IP or address, Ex: localhost");
    mainLayout->addWidget(edtConnectToServer);
    
    // Port: ______
    label = new QLabel("Port: ");
    mainLayout->addWidget(label);
    edtConnectToServerPort = new QLineEdit();
    // only accept integer.
    QIntValidator *intValidator = new QIntValidator();
    edtConnectToServerPort->setValidator(intValidator);
    edtConnectToServerPort->setPlaceholderText("Port, Ex:1883");
    mainLayout->addWidget(edtConnectToServerPort);

    // button Go!
    QPushButton *button = new QPushButton("Go!");
    connect(button, &QPushButton::clicked, this, &MainWindow::onGoButtonClicked);
    mainLayout->addWidget(button);
    
    mainLayout->addStretch();

    groupBox->setLayout(mainLayout);
    
    return groupBox;
}

QWidget *MainWindow::createClientBox() {
    QGroupBox *groupBox = new QGroupBox("Client");
    
    QHBoxLayout *mainLayout = new QHBoxLayout();
    
    // column 0
    QVBoxLayout *vLayout = new QVBoxLayout();
    
    // Server: ______
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Server: ");
    hLayout->addWidget(label);
    edtServer = new QLineEdit();
    edtServer->setPlaceholderText("Server IP or address");
    hLayout->addWidget(edtServer);
    vLayout->addLayout(hLayout);
    
    // Port: ______
    hLayout = new QHBoxLayout();
    label = new QLabel("Port: ");
    hLayout->addWidget(label);
    edtPort = new QLineEdit();
    // only accept integer.
    QIntValidator *intValidator = new QIntValidator();
    edtPort->setValidator(intValidator);
    edtPort->setPlaceholderText("Port");
    hLayout->addWidget(edtPort);
    vLayout->addLayout(hLayout);
    
    // Client ID: ______
    hLayout = new QHBoxLayout();
    label = new QLabel("Client ID: ");
    hLayout->addWidget(label);
    edtClientId = new QLineEdit();
    edtClientId->setPlaceholderText("Client ID");
    hLayout->addWidget(edtClientId);
    vLayout->addLayout(hLayout);
    
    // Username: ______
    hLayout = new QHBoxLayout();
    label = new QLabel("Username: ");
    hLayout->addWidget(label);
    edtUsername = new QLineEdit();
    edtUsername->setPlaceholderText("username");
    hLayout->addWidget(edtUsername);
    vLayout->addLayout(hLayout);
    
    // Password: ______
    hLayout = new QHBoxLayout();
    label = new QLabel("Password: ");
    hLayout->addWidget(label);
    edtPasswd = new QLineEdit();
    edtPasswd->setEchoMode(QLineEdit::Password);
    edtPasswd->setPlaceholderText("password");
    hLayout->addWidget(edtPasswd);
    vLayout->addLayout(hLayout);
    
    // []Anonymous Login
    cbAnonymousLogin = new QCheckBox("Anonymous Login");
    cbAnonymousLogin->setChecked(false);
    connect(cbAnonymousLogin, &QCheckBox::stateChanged, this, &MainWindow::onAnonymousLoginToggled);
    vLayout->addWidget(cbAnonymousLogin);
    vLayout->addStretch();
    
    mainLayout->addLayout(vLayout);
    
    // column 1
    vLayout = new QVBoxLayout();
    
    // Display Name: ______
    hLayout = new QHBoxLayout();
    label = new QLabel("Display Name: ");
    hLayout->addWidget(label);
    edtDisplayName = new QLineEdit();
    edtDisplayName->setPlaceholderText("Display Name");
    hLayout->addWidget(edtDisplayName);
    vLayout->addLayout(hLayout);
    
    // Client Description:
    label = new QLabel("Client Description: ");
    vLayout->addWidget(label);
    teDescription = new QTextEdit();
    teDescription->setPlaceholderText("Client Description");
    teDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vLayout->addWidget(teDescription);
    //vLayout->addStretch();
    
    mainLayout->addLayout(vLayout);
    
    // column 2
    vLayout = new QVBoxLayout();
    
    // buttons: generate client id, dummy client, run client
    QPushButton *button = new QPushButton("Generate Client ID");
    connect(button, &QPushButton::clicked, this, &MainWindow::onGenerateButtonClicked);
    vLayout->addWidget(button);
#if TEST // please refer to add_definitions(-DTEST=1) in CMakeLists.txt
    button = new QPushButton("Dummy Client");
    connect(button, &QPushButton::clicked, this, &MainWindow::onDummyClientButtonClicked);
    vLayout->addWidget(button);
#endif
    button = new QPushButton("Run Client");
    connect(button, &QPushButton::clicked, this, &MainWindow::onRunClientButtonClicked);
    vLayout->addWidget(button);
    vLayout->addStretch();
    
    mainLayout->addLayout(vLayout);
    
    groupBox->setLayout(mainLayout);
    
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
    gbClient->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(gbClient);

    central->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setCentralWidget(central);
}

void MainWindow::onGoButtonClicked()
{
    qDebug() << "go button clicked." << Qt::endl;
}

bool MainWindow::isClientIdExist(const std::string &clientId) const {
    bool ret = false;
    
    if (_clientWindows.find(clientId) != _clientWindows.end()) {
        ret = true;
    }
    
    return ret;
    
}

void MainWindow::onGenerateButtonClicked()
{
    const int len = 50;
    string clientId = Utils::generateString(len);
    while(isClientIdExist(clientId)) {
        clientId = Utils::generateString(len);
    }
    
    edtClientId->setText(QString::fromStdString(clientId));
}

void MainWindow::onRunClientButtonClicked()
{
    // validate / collect login info
    if (validateClientInput()) {
        // Ex.: tcp://localhost:1883
        string server = "tcp://"
                        + edtServer->text().toStdString()
                        + ":"
                        + edtPort->text().toStdString();
        string clientId = edtClientId->text().toStdString();
        if (isClientIdExist(clientId)) {
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
        
        // create a new client.
        ClientWindow *pClient = new ClientWindow(login);
        // we defined a customized signal (widgetClosed), to indicate if the user
        // close the client panel.
        // the client instance and client id will be destroyed after the user closed the client panel.
        connect(pClient, &ClientWindow::widgetClosed, this, [clientId, this]()->void {
            ClientWindow *client = _clientWindows[clientId];
            delete client;
            _clientWindows.erase(clientId);
        });
        // save the client id/instance pair.
        _clientWindows[clientId] = pClient;
        pClient->resize(800, 450);
        pClient->show();
    } else { // insufficient login info.
        QMessageBox msgBox;
        msgBox.setWindowTitle("Cannot create client.");
        msgBox.setText("Make sure Server, Port, Client ID, and Username are valid.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
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

#if TEST // please refer to add_definitions(-DTEST=1) in CMakeLists.txt
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
