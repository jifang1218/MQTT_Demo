//
//  ClientWindow.hpp
//  mqtttest0
//
//  Created by jifang on 2024-06-25.
//

#ifndef ClientWindow_hpp
#define ClientWindow_hpp

#include <QWidget>
#include <string>

namespace Fang {
class MQTTClient;
struct MQTTLoginInfo;
}

class QWidget;
class QLineEdit;
class QLayout;
class QPushButton;
class QLabel;
class QListWidget;

/**
 MQTT Client UI.
 */
class ClientWindow : public QWidget {
    Q_OBJECT
public:
    ClientWindow(const Fang::MQTTLoginInfo &loginInfo);
    ~ClientWindow();
    
private:
    // setup UI.
    void construct();
    QWidget *construct0_0();
    QWidget *construct0_1();
    QWidget *construct1_0();
    QWidget *construct1_1();
    QLayout *construct_rest();
    
    QLineEdit *edtPubTopic = nullptr;
    QPushButton *btnConnect = nullptr;
    QPushButton *btnDisconnect = nullptr;
    QPushButton *btnPublish = nullptr;
    QPushButton *btnSubscribe = nullptr;
    QLabel *lblStatus = nullptr;
    QListWidget *lwSubscribedTopics = nullptr;
    QListWidget *lwPublishedTopics = nullptr;
    QListWidget *lwReceivedMessages = nullptr;
    
private:
    Fang::MQTTClient *_client = nullptr;
    
private:
    enum class CallbackType {
        Connect = 0,
        Disconnect,
        Publish,
        Subscribe,
        Unsubscribe,
        MessageReceived
    };

private:
signals:
    // internally used signal.
    // we use it to notify user the client panel is closed.
    void widgetClosed();

protected:
    void closeEvent(QCloseEvent *event) override;
    
private:
signals:
    // internally used signal.
    // As callbacks from MQTT lib are running in non-ui thread, we use it to switch to UI thread to update UI components.
    void jump2UISignal(int type, int arg1, const QString &arg2, const QString &arg3, const void *client);
    
private slots:
    // internally used slot, please refer to the signal jump2UISignal.
    void jump2UISlot(int type, int arg1, const QString &arg2, const QString &arg3, const void *client);
    
    void onPubTopicSelectionChanged();
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    
private:
    void setupCallbacks();
    
    // these callbacks are running in non-ui thread, we will use signal/slot mechanism to switch to ui thread.
    // please refer to signal jump2UISignal and slot jump2UISlot. 
    void _onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &message);
    void _onSubscribeTopicCompleted(int errCode, const std::string &topic);
    void _onUnsubscribeTopicCompleted(int errCode, const std::string &topic);
    void _onConnectCompleted(int errCode, const Fang::MQTTClient *self);
    void _onDisconnectCompleted(int errCode, const Fang::MQTTClient *self);
    void _onMessageReceived(const std::string &topic, const std::string &message);
    
    // the following callbacks run in the UI thread.
    void onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &message);
    void onSubscribeTopicCompleted(int errCode, const std::string &topic);
    void onUnsubscribeTopicCompleted(int errCode, const std::string &topic);
    void onConnectCompleted(int errCode, const Fang::MQTTClient *self);
    void onDisconnectCompleted(int errCode, const Fang::MQTTClient *self);
    void onMessageReceived(const std::string &topic, const std::string &message);
};

#endif /* ClientWindow_hpp */
