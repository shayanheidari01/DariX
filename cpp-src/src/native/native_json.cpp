#include "darix/native/native.hpp"
#include <cctype>
#include <sstream>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

// Forward declarations for recursive parsing
static ObjectPtr parseValue(const std::string& json, size_t& pos);

static void skipWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;
}

static ObjectPtr parseString(const std::string& json, size_t& pos) {
    if (pos >= json.size() || json[pos] != '"') return makeError("expected '\"'");
    pos++; // skip opening quote
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\') {
            pos++;
            if (pos >= json.size()) return makeError("unexpected end of string escape");
            switch (json[pos]) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case '/':  result += '/'; break;
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case 'r':  result += '\r'; break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'u': {
                    pos++;
                    if (pos + 4 > json.size()) return makeError("incomplete unicode escape");
                    std::string hex = json.substr(pos, 4);
                    pos += 3; // will be incremented at loop end
                    uint32_t cp = 0;
                    for (char c : hex) {
                        cp <<= 4;
                        if (c >= '0' && c <= '9') cp += c - '0';
                        else if (c >= 'a' && c <= 'f') cp += c - 'a' + 10;
                        else if (c >= 'A' && c <= 'F') cp += c - 'A' + 10;
                        else return makeError("invalid hex in unicode escape");
                    }
                    // Simple UTF-8 encoding
                    if (cp < 0x80) result += static_cast<char>(cp);
                    else if (cp < 0x800) {
                        result += static_cast<char>(0xC0 | (cp >> 6));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        result += static_cast<char>(0xE0 | (cp >> 12));
                        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default: result += '\\'; result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        pos++;
    }
    if (pos >= json.size()) return makeError("unterminated string");
    pos++; // skip closing quote
    return newString(result);
}

static ObjectPtr parseNumber(const std::string& json, size_t& pos) {
    size_t start = pos;
    if (pos < json.size() && json[pos] == '-') pos++;
    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) pos++;
    bool isFloat = false;
    if (pos < json.size() && json[pos] == '.') {
        isFloat = true;
        pos++;
        while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) pos++;
    }
    if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
        isFloat = true;
        pos++;
        if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) pos++;
        while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) pos++;
    }
    std::string numStr = json.substr(start, pos - start);
    if (isFloat) {
        try { return newFloat(std::stod(numStr)); }
        catch (...) { return makeError("invalid number: " + numStr); }
    } else {
        try { return newInteger(std::stoll(numStr)); }
        catch (...) { return makeError("invalid number: " + numStr); }
    }
}

static ObjectPtr parseArray(const std::string& json, size_t& pos) {
    pos++; // skip '['
    skipWhitespace(json, pos);
    std::vector<ObjectPtr> elements;
    if (pos < json.size() && json[pos] == ']') { pos++; return newArray(elements); }
    while (pos < json.size()) {
        skipWhitespace(json, pos);
        auto val = parseValue(json, pos);
        if (val && val->type() == ObjectType::ERROR) return val;
        elements.push_back(val);
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') { pos++; continue; }
        if (pos < json.size() && json[pos] == ']') { pos++; break; }
        return makeError("expected ',' or ']' in array");
    }
    return newArray(elements);
}

static ObjectPtr parseObject(const std::string& json, size_t& pos) {
    pos++; // skip '{'
    skipWhitespace(json, pos);
    auto result = std::make_shared<Map>();
    if (pos < json.size() && json[pos] == '}') { pos++; return result; }
    while (pos < json.size()) {
        skipWhitespace(json, pos);
        auto key = parseString(json, pos);
        if (key && key->type() == ObjectType::ERROR) return key;
        skipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':') return makeError("expected ':' in object");
        pos++; // skip ':'
        skipWhitespace(json, pos);
        auto val = parseValue(json, pos);
        if (val && val->type() == ObjectType::ERROR) return val;
        result->pairs.push_back({key, val});
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') { pos++; continue; }
        if (pos < json.size() && json[pos] == '}') { pos++; break; }
        return makeError("expected ',' or '}' in object");
    }
    return result;
}

static ObjectPtr parseValue(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    if (pos >= json.size()) return makeError("unexpected end of input");
    char ch = json[pos];
    if (ch == '"') return parseString(json, pos);
    if (ch == '{') return parseObject(json, pos);
    if (ch == '[') return parseArray(json, pos);
    if (ch == 't') { pos += 4; return getTrue(); }
    if (ch == 'f') { pos += 5; return getFalse(); }
    if (ch == 'n') { pos += 4; return getNull(); }
    if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) return parseNumber(json, pos);
    return makeError(std::string("unexpected character: '") + ch + "'");
}

// Convert DariX object to JSON string
static std::string stringifyValue(ObjectPtr obj, int indent, int depth) {
    if (!obj) return "null";
    std::string pad(indent > 0 ? std::string(depth * indent, ' ') : "");
    std::string padInner(indent > 0 ? std::string((depth + 1) * indent, ' ') : "");
    std::string nl(indent > 0 ? "\n" : "");
    std::string comma(indent > 0 ? ",\n" : ",");

    switch (obj->type()) {
        case ObjectType::NULL_OBJ: return "null";
        case ObjectType::BOOLEAN: return std::dynamic_pointer_cast<Boolean>(obj)->value ? "true" : "false";
        case ObjectType::INTEGER: return std::to_string(std::dynamic_pointer_cast<Integer>(obj)->value);
        case ObjectType::FLOAT: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%g", std::dynamic_pointer_cast<Float>(obj)->value);
            return buf;
        }
        case ObjectType::STRING: {
            std::string s = std::dynamic_pointer_cast<String>(obj)->value;
            std::string result = "\"";
            for (char c : s) {
                switch (c) {
                    case '"':  result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\n': result += "\\n"; break;
                    case '\t': result += "\\t"; break;
                    case '\r': result += "\\r"; break;
                    case '\b': result += "\\b"; break;
                    case '\f': result += "\\f"; break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20) {
                            char buf[8];
                            std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(c)));
                            result += buf;
                        } else {
                            result += c;
                        }
                }
            }
            result += "\"";
            return result;
        }
        case ObjectType::ARRAY: {
            auto arr = std::dynamic_pointer_cast<Array>(obj);
            if (!arr || arr->elements.empty()) return "[]";
            std::string result = "[" + nl;
            for (size_t i = 0; i < arr->elements.size(); i++) {
                if (i > 0) result += comma;
                result += padInner + stringifyValue(arr->elements[i], indent, depth + 1);
            }
            result += nl + pad + "]";
            return result;
        }
        case ObjectType::MAP: {
            auto m = std::dynamic_pointer_cast<Map>(obj);
            if (!m || m->pairs.empty()) return "{}";
            std::string result = "{" + nl;
            for (size_t i = 0; i < m->pairs.size(); i++) {
                if (i > 0) result += comma;
                result += padInner + stringifyValue(m->pairs[i].first, indent, depth + 1) + ":" + (indent > 0 ? " " : "") + stringifyValue(m->pairs[i].second, indent, depth + 1);
            }
            result += nl + pad + "}";
            return result;
        }
        default: return obj->inspect();
    }
}

void initJsonModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    funcs["parse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("parse: expected 1 argument");
        if (!args[0] || args[0]->type() != ObjectType::STRING) return makeError("parse: argument must be string");
        std::string json = std::dynamic_pointer_cast<String>(args[0])->value;
        size_t pos = 0;
        auto result = parseValue(json, pos);
        skipWhitespace(json, pos);
        if (pos < json.size() && result && result->type() != ObjectType::ERROR) {
            return makeError("parse: unexpected trailing content at position " + std::to_string(pos));
        }
        return result;
    };

    funcs["stringify"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1 || args.size() > 2) return makeError("stringify: expected 1-2 arguments");
        int indent = 0;
        if (args.size() == 2) {
            auto indObj = std::dynamic_pointer_cast<Integer>(args[1]);
            if (indObj) indent = static_cast<int>(indObj->value);
        }
        return newString(stringifyValue(args[0], indent, 0));
    };

    funcs["is_valid"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_valid: expected 1 argument");
        if (!args[0] || args[0]->type() != ObjectType::STRING) return makeError("is_valid: argument must be string");
        std::string json = std::dynamic_pointer_cast<String>(args[0])->value;
        size_t pos = 0;
        auto result = parseValue(json, pos);
        skipWhitespace(json, pos);
        if (pos < json.size()) return newBoolean(false);
        return newBoolean(result && result->type() != ObjectType::ERROR);
    };

    Registry::instance().registerModule("json", funcs);
}

} // namespace darix::native
