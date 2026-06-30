#include "darix/native/native.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace darix::native {

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

static bool isString(ObjectPtr obj) {
    return obj && obj->type() == ObjectType::STRING;
}

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initStringModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    funcs["upper"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_upper: expected 1 argument");
        if (!isString(args[0])) return makeError("str_upper: argument must be string");
        std::string s = getString(args[0]);
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return newString(s);
    };

    funcs["lower"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_lower: expected 1 argument");
        if (!isString(args[0])) return makeError("str_lower: argument must be string");
        std::string s = getString(args[0]);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return newString(s);
    };

    funcs["trim"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_trim: expected 1 argument");
        if (!isString(args[0])) return makeError("str_trim: argument must be string");
        std::string s = getString(args[0]);
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return newString("");
        size_t end = s.find_last_not_of(" \t\n\r");
        return newString(s.substr(start, end - start + 1));
    };

    funcs["trim_left"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_trim_left: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_trim_left: arguments must be strings");
        std::string s = getString(args[0]);
        std::string cutset = getString(args[1]);
        size_t pos = s.find_first_not_of(cutset);
        return newString(pos == std::string::npos ? "" : s.substr(pos));
    };

    funcs["trim_right"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_trim_right: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_trim_right: arguments must be strings");
        std::string s = getString(args[0]);
        std::string cutset = getString(args[1]);
        size_t pos = s.find_last_not_of(cutset);
        return newString(pos == std::string::npos ? "" : s.substr(0, pos + 1));
    };

    funcs["split"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_split: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_split: arguments must be strings");
        std::string s = getString(args[0]);
        std::string sep = getString(args[1]);
        std::vector<ObjectPtr> parts;
        if (sep.empty()) {
            for (char c : s) parts.push_back(newString(std::string(1, c)));
        } else {
            size_t start = 0;
            size_t pos;
            while ((pos = s.find(sep, start)) != std::string::npos) {
                parts.push_back(newString(s.substr(start, pos - start)));
                start = pos + sep.size();
            }
            parts.push_back(newString(s.substr(start)));
        }
        return newArray(parts);
    };

    funcs["join"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_join: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("str_join: first argument must be array");
        if (!isString(args[1])) return makeError("str_join: second argument must be string");
        std::string sep = getString(args[1]);
        std::string result;
        for (size_t i = 0; i < arr->elements.size(); i++) {
            if (i > 0) result += sep;
            result += arr->elements[i]->inspect();
        }
        return newString(result);
    };

    funcs["replace"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("str_replace: expected 3 arguments");
        if (!isString(args[0]) || !isString(args[1]) || !isString(args[2]))
            return makeError("str_replace: arguments must be strings");
        std::string s = getString(args[0]);
        std::string old = getString(args[1]);
        std::string rep = getString(args[2]);
        if (old.empty()) return newString(s);
        size_t pos = 0;
        while ((pos = s.find(old, pos)) != std::string::npos) {
            s.replace(pos, old.size(), rep);
            pos += rep.size();
        }
        return newString(s);
    };

    funcs["contains"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_contains: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_contains: arguments must be strings");
        return newBoolean(getString(args[0]).find(getString(args[1])) != std::string::npos);
    };

    funcs["starts"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_starts: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_starts: arguments must be strings");
        std::string s = getString(args[0]);
        std::string prefix = getString(args[1]);
        if (prefix.size() > s.size()) return newBoolean(false);
        return newBoolean(s.compare(0, prefix.size(), prefix) == 0);
    };

    funcs["ends"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_ends: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_ends: arguments must be strings");
        std::string s = getString(args[0]);
        std::string suffix = getString(args[1]);
        if (suffix.size() > s.size()) return newBoolean(false);
        return newBoolean(s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0);
    };

    funcs["index"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_index: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_index: arguments must be strings");
        size_t pos = getString(args[0]).find(getString(args[1]));
        return newInteger(pos == std::string::npos ? -1 : static_cast<int64_t>(pos));
    };

    funcs["last_index"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_last_index: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("str_last_index: arguments must be strings");
        size_t pos = getString(args[0]).rfind(getString(args[1]));
        return newInteger(pos == std::string::npos ? -1 : static_cast<int64_t>(pos));
    };

    funcs["repeat"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("str_repeat: expected 2 arguments");
        if (!isString(args[0])) return makeError("str_repeat: first argument must be string");
        auto count = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!count) return makeError("str_repeat: second argument must be integer");
        if (count->value < 0) return makeError("str_repeat: count cannot be negative");
        std::string s = getString(args[0]);
        std::string result;
        result.reserve(s.size() * count->value);
        for (int64_t i = 0; i < count->value; i++) result += s;
        return newString(result);
    };

    funcs["reverse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_reverse: expected 1 argument");
        if (!isString(args[0])) return makeError("str_reverse: argument must be string");
        std::string s = getString(args[0]);
        std::reverse(s.begin(), s.end());
        return newString(s);
    };

    funcs["is_alpha"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_is_alpha: expected 1 argument");
        if (!isString(args[0])) return makeError("str_is_alpha: argument must be string");
        std::string s = getString(args[0]);
        if (s.empty()) return newBoolean(false);
        for (char c : s) { if (!std::isalpha(static_cast<unsigned char>(c))) return newBoolean(false); }
        return newBoolean(true);
    };

    funcs["is_digit"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("str_is_digit: expected 1 argument");
        if (!isString(args[0])) return makeError("str_is_digit: argument must be string");
        std::string s = getString(args[0]);
        if (s.empty()) return newBoolean(false);
        for (char c : s) { if (!std::isdigit(static_cast<unsigned char>(c))) return newBoolean(false); }
        return newBoolean(true);
    };

    funcs["is_space"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_space: expected 1 argument");
        if (!isString(args[0])) return makeError("is_space: argument must be string");
        std::string s = getString(args[0]);
        if (s.empty()) return newBoolean(false);
        for (char c : s) { if (!std::isspace(static_cast<unsigned char>(c))) return newBoolean(false); }
        return newBoolean(true);
    };

    // pad_left(str, width, pad_char) -> left-padded string
    funcs["pad_left"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("pad_left: expected 2-3 arguments");
        if (!isString(args[0])) return makeError("pad_left: first argument must be string");
        auto width = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!width) return makeError("pad_left: second argument must be integer");
        std::string s = getString(args[0]);
        char pad = ' ';
        if (args.size() == 3 && isString(args[2])) pad = getString(args[2])[0];
        if (static_cast<int64_t>(s.size()) >= width->value) return newString(s);
        return newString(std::string(width->value - s.size(), pad) + s);
    };

    // pad_right(str, width, pad_char) -> right-padded string
    funcs["pad_right"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("pad_right: expected 2-3 arguments");
        if (!isString(args[0])) return makeError("pad_right: first argument must be string");
        auto width = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!width) return makeError("pad_right: second argument must be integer");
        std::string s = getString(args[0]);
        char pad = ' ';
        if (args.size() == 3 && isString(args[2])) pad = getString(args[2])[0];
        if (static_cast<int64_t>(s.size()) >= width->value) return newString(s);
        return newString(s + std::string(width->value - s.size(), pad));
    };

    // slice(str, start, end) -> substring
    funcs["slice"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("slice: expected 2-3 arguments");
        if (!isString(args[0])) return makeError("slice: first argument must be string");
        auto startObj = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!startObj) return makeError("slice: second argument must be integer");
        std::string s = getString(args[0]);
        int64_t start = startObj->value;
        int64_t end = static_cast<int64_t>(s.size());
        if (args.size() == 3) {
            auto endObj = std::dynamic_pointer_cast<Integer>(args[2]);
            if (endObj) end = endObj->value;
        }
        if (start < 0) start = std::max(static_cast<int64_t>(0), static_cast<int64_t>(s.size()) + start);
        if (end < 0) end = std::max(static_cast<int64_t>(0), static_cast<int64_t>(s.size()) + end);
        start = std::min(start, static_cast<int64_t>(s.size()));
        end = std::min(end, static_cast<int64_t>(s.size()));
        if (start >= end) return newString("");
        return newString(s.substr(start, end - start));
    };

    // count(str, substr) -> number of occurrences
    funcs["count"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("count: expected 2 arguments");
        if (!isString(args[0]) || !isString(args[1])) return makeError("count: arguments must be strings");
        std::string s = getString(args[0]);
        std::string sub = getString(args[1]);
        if (sub.empty()) return newInteger(static_cast<int64_t>(s.size() + 1));
        int64_t count = 0;
        size_t pos = 0;
        while ((pos = s.find(sub, pos)) != std::string::npos) { count++; pos += sub.size(); }
        return newInteger(count);
    };

    // char_at(str, index) -> character at index as string
    funcs["char_at"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("char_at: expected 2 arguments");
        if (!isString(args[0])) return makeError("char_at: first argument must be string");
        auto idx = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!idx) return makeError("char_at: second argument must be integer");
        std::string s = getString(args[0]);
        int64_t i = idx->value;
        if (i < 0) i = static_cast<int64_t>(s.size()) + i;
        if (i < 0 || i >= static_cast<int64_t>(s.size())) return getNull();
        return newString(std::string(1, s[i]));
    };

    // to_title(str) -> title case
    funcs["to_title"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_title: expected 1 argument");
        if (!isString(args[0])) return makeError("to_title: argument must be string");
        std::string s = getString(args[0]);
        bool newWord = true;
        for (char& c : s) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                if (newWord) { c = std::toupper(static_cast<unsigned char>(c)); newWord = false; }
                else c = std::tolower(static_cast<unsigned char>(c));
            } else {
                newWord = true;
            }
        }
        return newString(s);
    };

    // chars(str) -> array of single-character strings
    funcs["chars"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("chars: expected 1 argument");
        if (!isString(args[0])) return makeError("chars: argument must be string");
        std::string s = getString(args[0]);
        std::vector<ObjectPtr> result;
        for (char c : s) result.push_back(newString(std::string(1, c)));
        return newArray(result);
    };

    // words(str) -> array of words (split by whitespace)
    funcs["words"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("words: expected 1 argument");
        if (!isString(args[0])) return makeError("words: argument must be string");
        std::string s = getString(args[0]);
        std::vector<ObjectPtr> result;
        std::istringstream iss(s);
        std::string word;
        while (iss >> word) result.push_back(newString(word));
        return newArray(result);
    };

    // lines(str) -> array of lines
    funcs["lines"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("lines: expected 1 argument");
        if (!isString(args[0])) return makeError("lines: argument must be string");
        std::string s = getString(args[0]);
        std::vector<ObjectPtr> result;
        std::istringstream iss(s);
        std::string line;
        while (std::getline(iss, line)) result.push_back(newString(line));
        return newArray(result);
    };

    // truncate(str, max_len, suffix) -> truncated string with suffix
    funcs["truncate"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("truncate: expected 2-3 arguments");
        if (!isString(args[0])) return makeError("truncate: first argument must be string");
        auto maxLen = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!maxLen) return makeError("truncate: second argument must be integer");
        std::string s = getString(args[0]);
        std::string suffix = "...";
        if (args.size() == 3 && isString(args[2])) suffix = getString(args[2]);
        if (static_cast<int64_t>(s.size()) <= maxLen->value) return newString(s);
        return newString(s.substr(0, maxLen->value - suffix.size()) + suffix);
    };

    // center(str, width, pad_char) -> centered string
    funcs["center"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("center: expected 2-3 arguments");
        if (!isString(args[0])) return makeError("center: first argument must be string");
        auto width = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!width) return makeError("center: second argument must be integer");
        std::string s = getString(args[0]);
        char pad = ' ';
        if (args.size() == 3 && isString(args[2])) pad = getString(args[2])[0];
        int64_t totalPad = width->value - static_cast<int64_t>(s.size());
        if (totalPad <= 0) return newString(s);
        int64_t leftPad = totalPad / 2;
        int64_t rightPad = totalPad - leftPad;
        return newString(std::string(leftPad, pad) + s + std::string(rightPad, pad));
    };

    // replace_first(str, old, new) -> string with first occurrence replaced
    funcs["replace_first"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("replace_first: expected 3 arguments");
        if (!isString(args[0]) || !isString(args[1]) || !isString(args[2]))
            return makeError("replace_first: arguments must be strings");
        std::string s = getString(args[0]);
        std::string old = getString(args[1]);
        std::string rep = getString(args[2]);
        if (old.empty()) return newString(s);
        size_t pos = s.find(old);
        if (pos != std::string::npos) s.replace(pos, old.size(), rep);
        return newString(s);
    };

    // is_empty(str) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        if (!isString(args[0])) return makeError("is_empty: argument must be string");
        return newBoolean(getString(args[0]).empty());
    };

    // starts_with(str, prefix) -> bool (alias for starts)
    funcs["starts_with"] = funcs["starts"];

    // ends_with(str, suffix) -> bool (alias for ends)
    funcs["ends_with"] = funcs["ends"];

    // to_int(str) -> integer
    funcs["to_int"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_int: expected 1 argument");
        if (!isString(args[0])) return makeError("to_int: argument must be string");
        try { return newInteger(std::stoll(getString(args[0]))); }
        catch (...) { return makeError("to_int: cannot convert to integer"); }
    };

    // to_float(str) -> float
    funcs["to_float"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_float: expected 1 argument");
        if (!isString(args[0])) return makeError("to_float: argument must be string");
        try { return newFloat(std::stod(getString(args[0]))); }
        catch (...) { return makeError("to_float: cannot convert to float"); }
    };

    // is_number(str) -> bool
    funcs["is_number"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_number: expected 1 argument");
        if (!isString(args[0])) return makeError("is_number: argument must be string");
        std::string s = getString(args[0]);
        if (s.empty()) return newBoolean(false);
        char* end;
        std::strtod(s.c_str(), &end);
        return newBoolean(end != s.c_str() && *end == '\0');
    };

    Registry::instance().registerModule("string", funcs);
}

} // namespace darix::native
