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

class ClientWindow : public QWidget {
    Q_OBJECT
public:
    ClientWindow(const Fang::MQTTLoginInfo &loginInfo);
    ~ClientWindow();
    
private:
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
    
private: signals:
    void jump2UISignal(int type, int arg1, const QString &arg2, const QString &arg3, const void *client);
    
private slots:
    void jump2UISlot(int type, int arg1, const QString &arg2, const QString &arg3, const void *client);
    void onPubTopicSelectionChanged();
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    
private:
    void setupCallbacks();
    void _onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &message);
    void _onSubscribeTopicCompleted(int errCode, const std::string &topic);
    void _onUnsubscribeTopicCompleted(int errCode, const std::string &topic);
    void _onConnectCompleted(int errCode, const Fang::MQTTClient *self);
    void _onDisconnectCompleted(int errCode, const Fang::MQTTClient *self);
    void _onMessageReceived(const std::string &topic, const std::string &message);
    void onPublishMessageCompleted(int errCode, const std::string &topic, const std::string &message);
    void onSubscribeTopicCompleted(int errCode, const std::string &topic);
    void onUnsubscribeTopicCompleted(int errCode, const std::string &topic);
    void onConnectCompleted(int errCode, const Fang::MQTTClient *self);
    void onDisconnectCompleted(int errCode, const Fang::MQTTClient *self);
    void onMessageReceived(const std::string &topic, const std::string &message);
};

#endif /* ClientWindow_hpp */
