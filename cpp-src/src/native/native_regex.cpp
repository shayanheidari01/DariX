#include "darix/native/native.hpp"
#include <regex>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

void initRegexModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // match(pattern, string) -> first match or null
    funcs["match"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("match: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::smatch m;
            if (std::regex_search(s, m, re)) {
                return newString(m[0].str());
            }
            return getNull();
        } catch (const std::regex_error& e) {
            return makeError(std::string("match: invalid regex: ") + e.what());
        }
    };

    // matches(pattern, string) -> array of all matches
    funcs["matches"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("matches: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::sregex_iterator it(s.begin(), s.end(), re);
            std::sregex_iterator end;
            std::vector<ObjectPtr> result;
            for (; it != end; ++it) {
                result.push_back(newString(it->str()));
            }
            return newArray(result);
        } catch (const std::regex_error& e) {
            return makeError(std::string("matches: invalid regex: ") + e.what());
        }
    };

    // groups(pattern, string) -> array of capture groups for first match, or null
    funcs["groups"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("groups: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::smatch m;
            if (std::regex_search(s, m, re)) {
                std::vector<ObjectPtr> result;
                for (size_t i = 1; i < m.size(); i++) {
                    result.push_back(m[i].matched ? newString(m[i].str()) : getNull());
                }
                return newArray(result);
            }
            return getNull();
        } catch (const std::regex_error& e) {
            return makeError(std::string("groups: invalid regex: ") + e.what());
        }
    };

    // named_groups(pattern, string) -> map of group name -> value
    funcs["named_groups"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("named_groups: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::smatch m;
            if (std::regex_search(s, m, re)) {
                auto result = std::make_shared<Map>();
                for (auto it = m.begin(); it != m.end(); ++it) {
                    if (it->matched) {
                        result->pairs.push_back({newString(it->str()), newString(it->str())});
                    }
                }
                return result;
            }
            return getNull();
        } catch (const std::regex_error& e) {
            return makeError(std::string("named_groups: invalid regex: ") + e.what());
        }
    };

    // test(pattern, string) -> bool
    funcs["test"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("test: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            return newBoolean(std::regex_search(getString(args[1]), re));
        } catch (const std::regex_error& e) {
            return makeError(std::string("test: invalid regex: ") + e.what());
        }
    };

    // replace(pattern, string, replacement) -> string with first match replaced
    funcs["replace"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("replace: expected 3 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::string rep = getString(args[2]);
            return newString(std::regex_replace(s, re, rep, std::regex_constants::format_first_only));
        } catch (const std::regex_error& e) {
            return makeError(std::string("replace: invalid regex: ") + e.what());
        }
    };

    // replace_all(pattern, string, replacement) -> string with all matches replaced
    funcs["replace_all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("replace_all: expected 3 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::string rep = getString(args[2]);
            return newString(std::regex_replace(s, re, rep));
        } catch (const std::regex_error& e) {
            return makeError(std::string("replace_all: invalid regex: ") + e.what());
        }
    };

    // split(pattern, string) -> array of parts
    funcs["split"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("split: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::sregex_token_iterator it(s.begin(), s.end(), re, -1);
            std::sregex_token_iterator end;
            std::vector<ObjectPtr> result;
            for (; it != end; ++it) {
                result.push_back(newString(it->str()));
            }
            return newArray(result);
        } catch (const std::regex_error& e) {
            return makeError(std::string("split: invalid regex: ") + e.what());
        }
    };

    // find(pattern, string) -> array of [start, length, text] for each match
    funcs["find"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::smatch m;
            std::vector<ObjectPtr> result;
            std::string searchStr = s;
            size_t offset = 0;
            while (std::regex_search(searchStr, m, re)) {
                auto matchObj = std::make_shared<Map>();
                int start = static_cast<int>(m.prefix().length()) + offset;
                matchObj->pairs.push_back({newString("start"), newInteger(start)});
                matchObj->pairs.push_back({newString("length"), newInteger(static_cast<int64_t>(m[0].str().size()))});
                matchObj->pairs.push_back({newString("text"), newString(m[0].str())});
                result.push_back(matchObj);
                offset += m.prefix().length() + m[0].str().size();
                searchStr = m.suffix().str();
            }
            return newArray(result);
        } catch (const std::regex_error& e) {
            return makeError(std::string("find: invalid regex: ") + e.what());
        }
    };

    // count(pattern, string) -> number of matches
    funcs["count"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("count: expected 2 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            std::sregex_iterator it(s.begin(), s.end(), re);
            std::sregex_iterator end;
            int64_t count = 0;
            for (; it != end; ++it) count++;
            return newInteger(count);
        } catch (const std::regex_error& e) {
            return makeError(std::string("count: invalid regex: ") + e.what());
        }
    };

    // escape(string) -> regex-safe string
    funcs["escape"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("escape: expected 1 argument");
        std::string s = getString(args[0]);
        std::string result;
        for (char c : s) {
            if (c == '.' || c == '*' || c == '+' || c == '?' || c == '(' || c == ')' ||
                c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' ||
                c == '^' || c == '$' || c == '|') {
                result += '\\';
            }
            result += c;
        }
        return newString(result);
    };

    // is_valid(pattern) -> bool
    funcs["is_valid"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_valid: expected 1 argument");
        try {
            std::regex re(getString(args[0]));
            return newBoolean(true);
        } catch (const std::regex_error&) {
            return newBoolean(false);
        }
    };

    // replace_with_fn(pattern, string, fn) -> string with fn called on each match
    funcs["replace_with_fn"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("replace_with_fn: expected 3 arguments");
        try {
            std::regex re(getString(args[0]));
            std::string s = getString(args[1]);
            ObjectPtr fn = args[2];

            std::string result;
            size_t lastEnd = 0;
            std::sregex_iterator it(s.begin(), s.end(), re);
            std::sregex_iterator end;
            for (; it != end; ++it) {
                result += s.substr(lastEnd, it->prefix().length());
                result += getString(callCallable(fn, {newString(it->str())}));
                lastEnd += it->prefix().length() + it->str().size();
            }
            result += s.substr(lastEnd);
            return newString(result);
        } catch (const std::regex_error& e) {
            return makeError(std::string("replace_with_fn: invalid regex: ") + e.what());
        }
    };

    Registry::instance().registerModule("regex", funcs);
}

} // namespace darix::native
