//
//  Utils.cpp
//  mqtttest0
//
//  Created by jifang on 2024-06-25.
//

#include "Utils.hpp"
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

namespace Fang {

std::string Utils::generateString(int len) {
    const int maxLen = 23; // MQTT 3.1.1, or 65535 for 5.0
    
    if (len > 23) {
        len = 23;
    }
    
    string text = "0123456789abcdefghijklmnopqrstuvwxyz";
    while (text.length() < len) {
        text += text;
    }
    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(text.begin(), text.end(), g);
    string ret = text.substr(0, len);
    
    return ret;
}

} // Fang
