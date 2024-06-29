//
//  Utils.hpp
//  mqtttest0
//
//  Created by jifang on 2024-06-25.
//

#ifndef Utils_hpp
#define Utils_hpp

#include <string>

namespace Fang {

/**
    Utility class
 */
class Utils {
public:
    /**
    Randomly generate a specific length of string. (0-9 and a-z)
     */
    static std::string generateString(int len);
};

} // Fang

#endif /* Utils_hpp */
