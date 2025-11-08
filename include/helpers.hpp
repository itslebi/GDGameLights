#ifndef GD_HELPERS_HPP
#define GD_HELPERS_HPP

#include <sstream>
#include <string>

namespace gdgamelights {
    inline bool isValidIPv4(const std::string& ip) {
        std::istringstream ss(ip);
        std::string token;
        int count = 0;

        while (std::getline(ss, token, '.')) {
            if (++count > 4) return false; // too many parts

            if (token.empty() || token.size() > 3) return false;

            // Check that all characters are digits
            for (char c : token) {
                if (!std::isdigit(c)) return false;
            }

            // Convert string to integer
            int num = std::stoi(token);
            if (num < 0 || num > 255) return false;

            // Prevent leading zeros like "01" except "0"
            if (token.size() > 1 && token[0] == '0') return false;
        }

        return count == 4; // must have exactly 4 parts
    }
}

#endif //GD_HELPERS_HPP