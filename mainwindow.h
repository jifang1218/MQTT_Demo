#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>
#include <string>

class QButtonGroup;
class QLineEdit;
class QTextEdit;
class QCheckBox;
class ServerWindow;
class ClientWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    // these three methods are used to create ui.
    void construct();
    QWidget *createServerBox();
    QWidget *createClientBox();
    
    // helpers
    bool validateClientInput();
    bool isClientIdExist(const std::string &clientId) const;
    
private:
    // server
    ServerWindow *pServerWindow = nullptr;
    QLineEdit *edtConnectToServer = nullptr;
    QLineEdit *edtConnectToServerPort = nullptr;
    
    // client
    QLineEdit *edtServer = nullptr;
    QLineEdit *edtPort = nullptr;
    QLineEdit *edtClientId = nullptr;
    QLineEdit *edtUsername = nullptr;
    QLineEdit *edtPasswd = nullptr;
    QLineEdit *edtDisplayName = nullptr;
    QTextEdit *teDescription = nullptr;
    QCheckBox *cbAnonymousLogin = nullptr;
    
    // store ClientWindows, will be destroyed when user close a client window.
    std::unordered_map<std::string, ClientWindow *> _clientWindows;
    
private slots:
    // server
    // connect to a MTQQ server
    void onGoButtonClicked();
    
    // client
    // randomly generate client id
    void onGenerateButtonClicked();
    // new client.
    void onRunClientButtonClicked();
    // login without authentication
    void onAnonymousLoginToggled(int state);
#if TEST // please refer to add_definitions(-DTEST=1) in CMakeLists.txt
    // a test method, auto-fill client login information.
    void onDummyClientButtonClicked();
#endif
    
};
#endif // MAINWINDOW_H
