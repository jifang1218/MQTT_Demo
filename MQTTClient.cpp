//
//  MQTTClient.cpp
//  mqtttest0
//
//  Created by jifang on 2024-06-21.
//

#include "MQTTClient.hpp"
#include "MQTTAsync.h"
#include "MQTTLoginInfo.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <QDebug>

namespace Fang {

using namespace std;

// callback functions used by Paho-MQTT-C
//PubMessageCallback onPublishMessageHandler;
static void onPubMessageSuccess(void* context, MQTTAsync_successData* response);
static void onPubMessageFailure(void* context, MQTTAsync_failureData* response);

//SubTopicCallback onSubscribeTopicHandler;
static void onSubTopicSuccess(void* context, MQTTAsync_successData* response);
static void onSubTopicFailure(void* context, MQTTAsync_failureData* response);

//UnsubTopicCallback onUnsubscribeTopicHandler;
static void onUnsubTopicSuccess(void* context, MQTTAsync_successData* response);
static void onUnsubTopicFailure(void* context, MQTTAsync_failureData* response);

//ConnectCallback onConnectHandler;
static void onConnectSuccess(void* context, MQTTAsync_successData* response);
static void onConnectFailure(void* context,  MQTTAsync_failureData* response);

//DisconnectCallback onDisconnectHandler;
static void onDisconnectSuccess(void* context, MQTTAsync_successData* response);
static void onDisconnectFailure(void* context,  MQTTAsync_failureData* response);

//MessageReceivedCallback onMessageReceivedHandler;
static int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);

static void onDeliveryCompleted(void *context, MQTTAsync_token dt);
static void onConnectionLost(void* context, char* cause);

// temporarily stored sending topics/messages
// used by subscribe/unsubscribe, publish actions.
struct Msg {
    std::string topic;
    std::string message;
};
static std::unordered_map<MQTTAsync_token, Msg> bufferedMsgs;

// indicate if the disconnect action is trigger by user or not.
bool _isUserInitDisconnectOnGoing = false;

MQTTClient::MQTTClient(const MQTTLoginInfo &login)
{
    _server = login.host;
    _clientId = login.clientId;
    _username = login.username;
    _password = login.password;
    _displayName = login.displayName;
    _description = login.description;
    
    _isAutoReconnect = false;
    
    MQTTAsync_createOptions opts = MQTTAsync_createOptions_initializer;
    int errCode = -1;
    // if you want MQTTCLIENT_PERSISTENCE_DEFAULT, you need to set a valid directory first.
    //const char* persistenceDir = "~/.mqttclient";
    //errCode = MQTTAsync_createWithOptions(&_client, _server.c_str(), _clientId.c_str(), MQTTCLIENT_PERSISTENCE_DEFAULT, (void*)persistenceDir, &opts);
    errCode = MQTTAsync_createWithOptions(&_client, _server.c_str(), _clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL, &opts);
    if (errCode != MQTTASYNC_SUCCESS) {
        qDebug() << "failed to create client, errcode: " << errCode << Qt::endl;
    }
}

MQTTClient::~MQTTClient()
{
    if (IsConnected()) {
        Disconnect();
    }
    MQTTAsync_destroy(&_client);
}

#pragma mark - Actions
int MQTTClient::Connect() {
    int ret = -1;
    
    do {
        if (IsConnected()) {
            if (this->onConnectHandler != nullptr) {
                this->onConnectHandler(MQTTASYNC_COMMAND_IGNORED, this);
            }
            break;
        }
        
        MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
        //    conn_opts.keepAliveInterval = 20;
        //    conn_opts.cleansession = 1;
        conn_opts.context = this;
        conn_opts.username = _username.c_str();
        conn_opts.password = _password.c_str();
        conn_opts.onSuccess = onConnectSuccess;
        conn_opts.onFailure = onConnectFailure;
        int errCode = -1;
        errCode = MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessageArrived, onDeliveryCompleted);
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onConnectHandler != nullptr) {
                this->onConnectHandler(errCode, this);
            }
            break;
        }
            
        errCode = MQTTAsync_connect(_client, &conn_opts);
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onConnectHandler != nullptr) {
                this->onConnectHandler(errCode, this);
            }
            break;
        }
        ret = 0;
    } while (0);
    
    return ret;
}

void MQTTClient::Disconnect() {
    do {
        if (!IsConnected()) {
            if (this->onDisconnectHandler != nullptr) {
                this->onDisconnectHandler(MQTTASYNC_COMMAND_IGNORED, this);
            }
            break;
        }
        
        _isUserInitDisconnectOnGoing = true;
        MQTTAsync_disconnectOptions disconnOpts = MQTTAsync_disconnectOptions_initializer;
        disconnOpts.context = this;
        disconnOpts.onSuccess = onDisconnectSuccess;
        disconnOpts.onFailure = onDisconnectFailure;
        
        int errCode = -1;
        errCode = MQTTAsync_disconnect(_client, &disconnOpts);
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onDisconnectHandler != nullptr) {
                this->onDisconnectHandler(errCode, this);
            }
            break;
        }
    } while (0);
}

bool MQTTClient::IsConnected() const {
    return MQTTAsync_isConnected(_client) != 0;
}

void MQTTClient::AddPubTopic(const std::string &pubTopic) {
    auto pos = find(_pubTopics.begin(), _pubTopics.end(), pubTopic);
    if (pos == _pubTopics.end()) {
        _pubTopics.push_back(pubTopic);
    }
}

void MQTTClient::RemovePubTopic(const std::string &pubTopic) {
    auto pos = find(_pubTopics.begin(), _pubTopics.end(), pubTopic);
    if (pos != _pubTopics.end()) {
        _pubTopics.erase(pos);
    }
}

void MQTTClient::AddSubTopic(const std::string &subTopic) {
    auto pos = find(_subTopics.begin(), _subTopics.end(), subTopic);
    if (pos == _subTopics.end()) {
        _subTopics.push_back(subTopic);
    }
}

void MQTTClient::RemoveSubTopic(const std::string &subTopic) {
    auto pos = find(_subTopics.begin(), _subTopics.end(), subTopic);
    if (pos != _subTopics.end()) {
        _subTopics.erase(pos);
    }
}

void MQTTClient::PubMessageForTopic(const std::string &topic, const std::string &message, int qos) {
    do {
        if (!IsConnected()) {
            if (this->onPublishMessageHandler != nullptr) {
                this->onPublishMessageHandler(MQTTASYNC_DISCONNECTED, topic, message);
            }
            break;
        }
    
        MQTTAsync_message pub = MQTTAsync_message_initializer;
        pub.payload = (void *)message.c_str();
        pub.payloadlen = (int)message.length();
        pub.qos = qos;
        MQTTAsync_responseOptions respOpts = MQTTAsync_responseOptions_initializer;
        respOpts.context = this;
        respOpts.onFailure = onPubMessageFailure;
        respOpts.onSuccess = onPubMessageSuccess;
        int errCode = MQTTAsync_sendMessage(_client, topic.c_str(), &pub, &respOpts);
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onPublishMessageHandler != nullptr) {
                this->onPublishMessageHandler(errCode, topic, message);
            }
            break;
        }
     
        // buffer message, will handle it in Publish callback function. 
        Msg msg = {topic, message};
        bufferedMsgs[respOpts.token] = msg;
    } while (0);
}

void MQTTClient::SubTopic(const std::string &topic, int qos) {
    do {
        if (find(_subTopics.begin(), _subTopics.end(), topic) != _subTopics.end()) {
            if (this->onSubscribeTopicHandler != nullptr) {
                this->onSubscribeTopicHandler(MQTTASYNC_COMMAND_IGNORED, topic);
            }
            break;
        }
        
        if (!IsConnected()) {
            if (this->onSubscribeTopicHandler != nullptr) {
                this->onSubscribeTopicHandler(MQTTASYNC_DISCONNECTED, topic);
            }
            break;
        }
        MQTTAsync_responseOptions respOpts = MQTTAsync_responseOptions_initializer;
        respOpts.context = this;
        respOpts.onFailure = onSubTopicFailure;
        respOpts.onSuccess = onSubTopicSuccess;
        int errCode = MQTTAsync_subscribe(_client, topic.c_str(), qos, &respOpts);
        
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onSubscribeTopicHandler != nullptr) {
                this->onSubscribeTopicHandler(errCode, topic);
            }
            break;
        }
        
        // buffer message, will handle it in subscribe callback function.
        Msg msg = {topic, ""};
        bufferedMsgs[respOpts.token] = msg;
    } while (0);
}

void MQTTClient::UnsubTopic(const std::string &topic) {
    do {
        if (find(_subTopics.begin(), _subTopics.end(), topic) == _subTopics.end()) {
            if (this->onUnsubscribeTopicHandler != nullptr) {
                this->onUnsubscribeTopicHandler(MQTTASYNC_COMMAND_IGNORED, topic);
            }
            break;
        }
        
        if (!IsConnected()) {
            if (this->onUnsubscribeTopicHandler != nullptr) {
                this->onUnsubscribeTopicHandler(MQTTASYNC_DISCONNECTED, topic);
            }
            break;
        }
        
        MQTTAsync_responseOptions respOpts = MQTTAsync_responseOptions_initializer;
        respOpts.context = this;
        respOpts.onFailure = onUnsubTopicFailure;
        respOpts.onSuccess = onUnsubTopicSuccess;
        int errCode = MQTTAsync_unsubscribe(_client, topic.c_str(), &respOpts);
        if (errCode != MQTTASYNC_SUCCESS) {
            if (this->onUnsubscribeTopicHandler != nullptr) {
                this->onUnsubscribeTopicHandler(errCode, topic);
            }
            break;
        }
        
        // buffer message, will handle it in unsubscribe callback function.
        Msg msg = {topic, ""};
        bufferedMsgs[respOpts.token] = msg;
    } while (0);
}

int MQTTClient::Reconnect() {
    MQTTAsync_reconnect(static_cast<MQTTAsync>(_client));
}

#pragma mark - internal Paho-MQTT-C callbacks
void onPubMessageSuccess(void* context, MQTTAsync_successData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    string message = bufferedMsgs[response->token].message;
    bufferedMsgs.erase(response->token);
    
    self->AddPubTopic(topic);
    
    if (self->onPublishMessageHandler != nullptr) {
        self->onPublishMessageHandler(0, topic, message);
    }
}

void onPubMessageFailure(void* context, MQTTAsync_failureData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    string message = bufferedMsgs[response->token].message;
    bufferedMsgs.erase(response->token);
    
    if (self->onPublishMessageHandler != nullptr) {
        self->onPublishMessageHandler(response->code, topic, message);
    }
}

void onSubTopicSuccess(void* context, MQTTAsync_successData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    bufferedMsgs.erase(response->token);
    
    self->AddSubTopic(topic);
    
    if (self->onSubscribeTopicHandler != nullptr) {
        self->onSubscribeTopicHandler(0, topic);
    }
}

void onSubTopicFailure(void* context, MQTTAsync_failureData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    bufferedMsgs.erase(response->token);
    
    if (self->onSubscribeTopicHandler != nullptr) {
        self->onSubscribeTopicHandler(response->code, topic);
    }
}

void onUnsubTopicSuccess(void* context, MQTTAsync_successData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    bufferedMsgs.erase(response->token);
    
    self->RemoveSubTopic(topic);
    
    if (self->onUnsubscribeTopicHandler != nullptr) {
        self->onUnsubscribeTopicHandler(0, topic);
    }
}

void onUnsubTopicFailure(void* context, MQTTAsync_failureData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    
    string topic = bufferedMsgs[response->token].topic;
    bufferedMsgs.erase(response->token);
    
    if (self->onUnsubscribeTopicHandler != nullptr) {
        self->onUnsubscribeTopicHandler(response->code, topic);
    }
}

void onConnectSuccess(void* context, MQTTAsync_successData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    if (self->onConnectHandler != nullptr) {
        self->onConnectHandler(0, self);
    }
}

void onConnectFailure(void* context,  MQTTAsync_failureData* response) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    if (self->onConnectHandler != nullptr) {
        self->onConnectHandler(response->code, self);
    }
}

void onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
    _isUserInitDisconnectOnGoing = false;
    MQTTClient *self = static_cast<MQTTClient *>(context);
    if (self->onDisconnectHandler != nullptr) {
        self->onDisconnectHandler(0, self);
    }
}

void onDisconnectFailure(void* context,  MQTTAsync_failureData* response) {
    _isUserInitDisconnectOnGoing = false;
    MQTTClient *self = static_cast<MQTTClient *>(context);
    if (self->onDisconnectHandler != nullptr) {
        self->onDisconnectHandler(response->code, self);
    }
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    string topic(topicName, topicLen);
    string msg(static_cast<const char *>(message->payload), message->payloadlen);
    if (self->onMessageReceivedHandler != nullptr) {
        self->onMessageReceivedHandler(topic, msg);
    }
}

void onDeliveryCompleted(void *context, MQTTAsync_token dt) {
}

void onConnectionLost(void* context, char* cause) {
    MQTTClient *self = static_cast<MQTTClient *>(context);
    if (self->IsAutoReconnectEnabled()
        && !_isUserInitDisconnectOnGoing) {
        self->Reconnect();
    }
    
    if (self->onDisconnectHandler != nullptr) {
        self->onDisconnectHandler(0, self);
    }
}

#pragma mark - getters/setters
const std::vector<std::string> &MQTTClient::GetPubTopics() const {
    return _pubTopics;
}

const std::vector<std::string> &MQTTClient::GetSubTopics() const {
    return _subTopics;
}

bool MQTTClient::IsAutoReconnectEnabled() const {
    return _isAutoReconnect;
}

void MQTTClient::SetAutoReconnectEnabled(bool enable) {
    _isAutoReconnect = enable;
}

std::string MQTTClient::GetServer() const {
    return _server;
}

std::string MQTTClient::GetClientId() const {
    return _clientId;
}

std::string MQTTClient::GetDescription() const {
    return _description;
}

void MQTTClient::SetDescription(const std::string &description) {
    _description = description;
}

std::string MQTTClient::GetDisplayName() const {
    string ret = _displayName;
    if (ret == "") {
        ret = _clientId;
    }
    
    return ret;
}

void MQTTClient::SetDisplayName(const std::string &displayName) {
    _displayName = displayName;
}

} // Fang
