//
//  MQTTClient.hpp
//  mqtttest0
//
//  Created by jifang on 2024-06-21.
//

#ifndef MQTTClient_hpp
#define MQTTClient_hpp

#include <string>
#include <vector>
#include <functional>

namespace Fang {

struct MQTTLoginInfo;

/**
 A MQTT Client, it uses Paho-MQTT-C
 */
class MQTTClient {
public:
    using PubMessageCallback = std::function<void(int errCode,
                                                  const std::string &topic,
                                                  const std::string &data)>;
    
    using SubTopicCallback = std::function<void(int errCode,
                                                const std::string &topic)>;
    
    using UnsubTopicCallback = std::function<void(int errCode,
                                                  const std::string &topic)>;
    
    using ConnectCallback = std::function<void(int errCode,
                                               const MQTTClient *self)>;
    
    using DisconnectCallback = std::function<void(int errCode,
                                                  const MQTTClient *self)>;
    
    using MessageReceivedCallback = std::function<void(const std::string &topic,
                                                       const std::string &data)>;
    
    // all the callbacks except MessageReceivedCallback accept an errCode to indicate
    // if the operation succeeded (with value 0) or not (with a negative value).
    // check MQTTASYNC_SUCCESS for more detailed information about errCode. 
    PubMessageCallback onPublishMessageHandler;
    SubTopicCallback onSubscribeTopicHandler;
    UnsubTopicCallback onUnsubscribeTopicHandler;
    ConnectCallback onConnectHandler;
    DisconnectCallback onDisconnectHandler;
    MessageReceivedCallback onMessageReceivedHandler;
    
public:
    MQTTClient(const MQTTLoginInfo &login);
    ~MQTTClient();
    
public:
    int Connect();
    void Disconnect();
    bool IsConnected() const;
    int Reconnect();
    bool IsAutoReconnectEnabled() const;
    void SetAutoReconnectEnabled(bool enable);
    
    // these four methods are used for maintaining the internal topic related data.
    // they are used in MQTT callbacks.
    // due to the accessbility issue they have to be public.
    // ugly design.
    void AddPubTopic(const std::string &pubTopic);
    void RemovePubTopic(const std::string &pubTopic);
    void AddSubTopic(const std::string &subTopic);
    void RemoveSubTopic(const std::string &subTopic);
    
    // actions
    void PubMessageForTopic(const std::string &topic, const std::string &message, int qos);
    void SubTopic(const std::string &topic, int qos);
    void UnsubTopic(const std::string &topic);
    
    // internal topic related data.
    // recent published topics.
    const std::vector<std::string> &GetPubTopics() const;
    // currently subscribed topics.
    const std::vector<std::string> &GetSubTopics() const;
    
public:
    // getters and setters
    std::string GetServer() const;
    std::string GetClientId() const;
    std::string GetDescription() const;
    void SetDescription(const std::string &description);
    std::string GetDisplayName() const;
    void SetDisplayName(const std::string &displayName);

private:
    // from MQTTAsync: typedef void* MQTTAsync;
    void *_client = nullptr;
    std::string _server;
    std::string _clientId;
    std::string _username;
    std::string _password;
    bool _isAutoReconnect = false;
    std::string _description;
    std::string _displayName;
    
    // internal topic related data.
    // recent published topics.
    std::vector<std::string> _pubTopics;
    // currently subscribed topics.
    std::vector<std::string> _subTopics;
};

} // Fang

#endif /* MQTTClient_hpp */
