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
    
    void AddPubTopic(const std::string &pubTopic);
    void RemovePubTopic(const std::string &pubTopic);
    void AddSubTopic(const std::string &subTopic);
    void RemoveSubTopic(const std::string &subTopic);
    
    void PubMessageForTopic(const std::string &topic, const std::string &message, int qos);
    void SubTopic(const std::string &topic, int qos);
    void UnsubTopic(const std::string &topic);
    
    const std::vector<std::string> &GetPubTopics() const;
    const std::vector<std::string> &GetSubTopics() const;
    
public:
    std::string GetServer() const;
    std::string GetClientId() const;
    int GetQOS() const;
    std::string GetDescription() const;
    void SetDescription(const std::string &description);
    std::string GetDisplayName() const;
    void SetDisplayName(const std::string &displayName);

private:
    // MQTTAsync _client;
    // MQTTAsync is void *
    void *_client = nullptr;
    std::string _server;
    std::string _clientId;
    std::string _username;
    std::string _password;
    bool _isAutoReconnect = false;
    
    std::vector<std::string> _pubTopics;
    std::vector<std::string> _subTopics;
    std::string _description;
    std::string _displayName;
};

} // Fang

#endif /* MQTTClient_hpp */
