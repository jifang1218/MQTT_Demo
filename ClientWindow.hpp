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
#include <list>

namespace Fang {
class MQTTClient;
struct MQTTLoginInfo;
}

class QWidget;

class ClientWindow : public QWidget {
public:
    ClientWindow(const Fang::MQTTLoginInfo &loginInfo);
    ~ClientWindow();
    
public:
    std::string GetClientId() const;
    
private:
    void construct();
    QWidget *construct0_0();
    QWidget *construct0_1();
    QWidget *construct1_0();
    QWidget *construct1_1();
    
private:
    Fang::MQTTClient *_client = nullptr;
    
private slots:
};

#endif /* ClientWindow_hpp */
