#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>

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
    void construct();
    QWidget *createServerBox();
    QWidget *createClientBox();
    bool validateClientInput();
    
private:
    // server
    QButtonGroup *serverButtonGroup = nullptr; // server group buttons
    ServerWindow *pServerWindow = nullptr;
    
    // client
    QLineEdit *edtConnectToServer = nullptr;
    QLineEdit *edtConnectToServerPort = nullptr;
    
    QLineEdit *edtServer = nullptr;
    QLineEdit *edtPort = nullptr;
    QLineEdit *edtClientId = nullptr;
    QLineEdit *edtUsername = nullptr;
    QLineEdit *edtPasswd = nullptr;
    QLineEdit *edtDisplayName = nullptr;
    QTextEdit *teDescription = nullptr;
    QCheckBox *cbAnonymousLogin = nullptr;
    
    std::unordered_map<std::string, ClientWindow *> _clients;
    
private slots:
    // server
    void onServerCreationButtonToggled(int id, bool checked);
    void onGoButtonClicked();
    
    // client
    void onGenerateButtonClicked();
    void onRunClientButtonClicked();
    void onAnonymousLoginToggled(int state);
    
#if TEST
    void onDummyClientButtonClicked();
#endif
    
};
#endif // MAINWINDOW_H
