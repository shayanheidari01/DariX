#include "darix/native/native.hpp"
#include <algorithm>
#include <sstream>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

static std::string toHex(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; i++) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", data[i]);
        result += buf;
    }
    return result;
}

static std::string toHexUpper(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; i++) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02X", data[i]);
        result += buf;
    }
    return result;
}

// Base64 encode
static std::string base64Encode(const std::string& data) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);
    unsigned char buf[3];
    size_t len = 0;
    for (unsigned char c : data) {
        buf[len++] = c;
        if (len == 3) {
            result += table[buf[0] >> 2];
            result += table[((buf[0] & 0x03) << 4) | (buf[1] >> 4)];
            result += table[((buf[1] & 0x0F) << 2) | (buf[2] >> 6)];
            result += table[buf[2] & 0x3F];
            len = 0;
        }
    }
    if (len) {
        result += table[buf[0] >> 2];
        if (len == 1) result += table[(buf[0] & 0x03) << 4];
        else result += table[((buf[0] & 0x03) << 4) | (buf[1] >> 4)];
        if (len == 2) result += table[(buf[1] & 0x0F) << 2];
        else result += '=';
        result += '=';
    }
    return result;
}

// Base64 decode
static std::string base64Decode(const std::string& data) {
    std::string result;
    result.reserve(data.size() * 3 / 4);
    uint32_t buf = 0;
    int bits = 0;
    for (unsigned char c : data) {
        if (c == '=' || c == '\n' || c == '\r') continue;
        int val = 0;
        if (c >= 'A' && c <= 'Z') val = c - 'A';
        else if (c >= 'a' && c <= 'z') val = c - 'a' + 26;
        else if (c >= '0' && c <= '9') val = c - '0' + 52;
        else if (c == '+') val = 62;
        else if (c == '/') val = 63;
        else continue;
        buf = (buf << 6) | val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            result += static_cast<char>((buf >> bits) & 0xFF);
        }
    }
    return result;
}

// Base32 encode
static std::string base32Encode(const std::string& data) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::string result;
    result.reserve(((data.size() + 4) / 5) * 8);
    unsigned char buf[5];
    size_t len = 0;
    for (unsigned char c : data) {
        buf[len++] = c;
        if (len == 5) {
            result += table[(buf[0] >> 3) & 0x1F];
            result += table[((buf[0] & 0x07) << 2) | ((buf[1] >> 6) & 0x03)];
            result += table[(buf[1] >> 1) & 0x1F];
            result += table[((buf[1] & 0x01) << 4) | ((buf[2] >> 4) & 0x0F)];
            result += table[((buf[2] & 0x0F) << 1) | ((buf[3] >> 7) & 0x01)];
            result += table[(buf[3] >> 2) & 0x1F];
            result += table[((buf[3] & 0x03) << 3) | ((buf[4] >> 5) & 0x07)];
            result += table[buf[4] & 0x1F];
            len = 0;
        }
    }
    if (len) {
        for (size_t i = len; i < 5; i++) buf[i] = 0;
        result += table[(buf[0] >> 3) & 0x1F];
        result += table[((buf[0] & 0x07) << 2) | ((buf[1] >> 6) & 0x03)];
        if (len >= 1) result += table[(buf[1] >> 1) & 0x1F];
        if (len >= 2) result += table[((buf[1] & 0x01) << 4) | ((buf[2] >> 4) & 0x0F)];
        if (len >= 3) result += table[((buf[2] & 0x0F) << 1) | ((buf[3] >> 7) & 0x01)];
        if (len >= 4) result += table[(buf[3] >> 2) & 0x1F];
        if (len == 4) result += table[((buf[3] & 0x03) << 3)];
        while (result.size() % 8 != 0) result += '=';
    }
    return result;
}

// Base32 decode
static std::string base32Decode(const std::string& data) {
    std::string result;
    uint32_t buf = 0;
    int bits = 0;
    for (unsigned char c : data) {
        if (c == '=' || c == '\n' || c == '\r' || c == ' ') continue;
        int val = -1;
        if (c >= 'A' && c <= 'Z') val = c - 'A';
        else if (c >= '2' && c <= '7') val = c - '2' + 26;
        else if (c >= 'a' && c <= 'z') val = c - 'a';
        if (val < 0) continue;
        buf = (buf << 5) | val;
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            result += static_cast<char>((buf >> bits) & 0xFF);
        }
    }
    return result;
}

void initEncodingModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // Base64
    funcs["base64_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base64_encode: expected 1 argument");
        return newString(base64Encode(getString(args[0])));
    };
    funcs["base64_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base64_decode: expected 1 argument");
        return newString(base64Decode(getString(args[0])));
    };

    // Base32
    funcs["base32_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base32_encode: expected 1 argument");
        return newString(base32Encode(getString(args[0])));
    };
    funcs["base32_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base32_decode: expected 1 argument");
        return newString(base32Decode(getString(args[0])));
    };

    // Hex
    funcs["hex_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hex_encode: expected 1 argument");
        std::string data = getString(args[0]);
        return newString(toHex(reinterpret_cast<const unsigned char*>(data.c_str()), data.size()));
    };
    funcs["hex_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hex_decode: expected 1 argument");
        std::string hex = getString(args[0]);
        if (hex.size() % 2 != 0) return makeError("hex_decode: invalid hex string");
        std::string result;
        result.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            char byte = 0;
            for (int j = 0; j < 2; j++) {
                char c = hex[i + j];
                byte <<= 4;
                if (c >= '0' && c <= '9') byte |= c - '0';
                else if (c >= 'a' && c <= 'f') byte |= c - 'a' + 10;
                else if (c >= 'A' && c <= 'F') byte |= c - 'A' + 10;
                else return makeError("hex_decode: invalid hex character");
            }
            result += byte;
        }
        return newString(result);
    };
    funcs["hex_encode_upper"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hex_encode_upper: expected 1 argument");
        std::string data = getString(args[0]);
        return newString(toHexUpper(reinterpret_cast<const unsigned char*>(data.c_str()), data.size()));
    };

    // URL encode/decode
    funcs["url_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("url_encode: expected 1 argument");
        std::string result;
        for (unsigned char c : getString(args[0])) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') result += static_cast<char>(c);
            else { char buf[4]; std::snprintf(buf, sizeof(buf), "%%%02X", c); result += buf; }
        }
        return newString(result);
    };
    funcs["url_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("url_decode: expected 1 argument");
        std::string data = getString(args[0]), result;
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i] == '%' && i + 2 < data.size()) {
                char byte = 0;
                for (int j = 1; j <= 2; j++) {
                    char c = data[i + j]; byte <<= 4;
                    if (c >= '0' && c <= '9') byte |= c - '0';
                    else if (c >= 'a' && c <= 'f') byte |= c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F') byte |= c - 'A' + 10;
                    else { result += data[i]; goto next; }
                }
                result += byte; i += 2;
            } else if (data[i] == '+') result += ' ';
            else result += data[i];
            next:;
        }
        return newString(result);
    };

    // HTML encode/decode
    funcs["html_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("html_encode: expected 1 argument");
        std::string s = getString(args[0]), result;
        for (char c : s) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\'': result += "&#39;"; break;
                default: result += c;
            }
        }
        return newString(result);
    };
    funcs["html_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("html_decode: expected 1 argument");
        std::string s = getString(args[0]), result;
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '&' && i + 1 < s.size()) {
                auto end = s.find(';', i + 1);
                if (end != std::string::npos) {
                    std::string entity = s.substr(i + 1, end - i - 1);
                    if (entity == "amp") result += '&';
                    else if (entity == "lt") result += '<';
                    else if (entity == "gt") result += '>';
                    else if (entity == "quot") result += '"';
                    else if (entity == "apos") result += '\'';
                    else { result += s.substr(i, end - i + 1); }
                    i = end;
                    continue;
                }
            }
            result += s[i];
        }
        return newString(result);
    };

    // Binary string
    funcs["binary_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("binary_encode: expected 1 argument");
        std::string data = getString(args[0]);
        std::string result;
        for (size_t idx = 0; idx < data.size(); idx++) {
            unsigned char c = static_cast<unsigned char>(data[idx]);
            for (int j = 7; j >= 0; j--) result += ((c >> j) & 1) ? '1' : '0';
            if (idx + 1 < data.size()) result += ' ';
        }
        return newString(result);
    };
    funcs["binary_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("binary_decode: expected 1 argument");
        std::string bits = getString(args[0]);
        // Remove spaces
        bits.erase(std::remove(bits.begin(), bits.end(), ' '), bits.end());
        if (bits.size() % 8 != 0) return makeError("binary_decode: invalid binary string length");
        std::string result;
        for (size_t i = 0; i < bits.size(); i += 8) {
            char byte = 0;
            for (int j = 0; j < 8; j++) {
                byte <<= 1;
                if (bits[i + j] == '1') byte |= 1;
            }
            result += byte;
        }
        return newString(result);
    };

    // Octal encode/decode
    funcs["octal_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("octal_encode: expected 1 argument");
        std::string data = getString(args[0]);
        std::string result;
        for (size_t idx = 0; idx < data.size(); idx++) {
            unsigned char c = static_cast<unsigned char>(data[idx]);
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%03o", c);
            if (idx > 0) result += ' ';
            result += buf;
        }
        return newString(result);
    };
    funcs["octal_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("octal_decode: expected 1 argument");
        std::string s = getString(args[0]);
        std::string result;
        std::istringstream iss(s);
        std::string token;
        while (iss >> token) {
            result += static_cast<char>(std::stoi(token, nullptr, 8));
        }
        return newString(result);
    };

    // Caesar cipher
    funcs["caesar_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("caesar_encode: expected 2 arguments");
        int shift = static_cast<int>([&]() -> int64_t { if (auto i = std::dynamic_pointer_cast<Integer>(args[1])) return i->value; return 0; }());
        std::string s = getString(args[0]), result;
        for (char c : s) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                char base = std::isupper(static_cast<unsigned char>(c)) ? 'A' : 'a';
                result += static_cast<char>((c - base + shift + 26) % 26 + base);
            } else {
                result += c;
            }
        }
        return newString(result);
    };
    funcs["caesar_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("caesar_decode: expected 2 arguments");
        int shift = static_cast<int>([&]() -> int64_t { if (auto i = std::dynamic_pointer_cast<Integer>(args[1])) return i->value; return 0; }());
        std::string s = getString(args[0]), result;
        for (char c : s) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                char base = std::isupper(static_cast<unsigned char>(c)) ? 'A' : 'a';
                result += static_cast<char>((c - base - shift + 26) % 26 + base);
            } else {
                result += c;
            }
        }
        return newString(result);
    };

    // Rot13 (caesar with shift 13)
    funcs["rot13"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("rot13: expected 1 argument");
        std::string s = getString(args[0]), result;
        for (char c : s) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                char base = std::isupper(static_cast<unsigned char>(c)) ? 'A' : 'a';
                result += static_cast<char>((c - base + 13) % 26 + base);
            } else {
                result += c;
            }
        }
        return newString(result);
    };

    // XOR encode/decode (symmetric)
    funcs["xor_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("xor_encode: expected 2 arguments");
        std::string data = getString(args[0]);
        std::string key = getString(args[1]);
        if (key.empty()) return makeError("xor_encode: key must not be empty");
        std::string result;
        for (size_t i = 0; i < data.size(); i++) {
            result += static_cast<char>(data[i] ^ key[i % key.size()]);
        }
        return newString(result);
    };

    Registry::instance().registerModule("encoding", funcs);
}

} // namespace darix::native
