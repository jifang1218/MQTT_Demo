//
//  MQTTLoginInfo.hpp
//  mqtttest0
//
//  Created by jifang on 2024-06-21.
//

#ifndef MQTTLoginInfo_hpp
#define MQTTLoginInfo_hpp

#include <string>

namespace Fang {

struct MQTTLoginInfo {
    std::string host;
    std::string clientId;
    std::string username;
    std::string password;
    std::string displayName;
    std::string description;
    
    MQTTLoginInfo();
};

} // Fang

#endif
