#include "darix/native/native.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

void initIoModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // print(args...) -> null, prints to stdout with newline
    funcs["print"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::string out;
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) out += " ";
            out += args[i]->inspect();
        }
        std::printf("%s\n", out.c_str());
        return getNull();
    };

    // print_no_newline(args...) -> null, prints to stdout without newline
    funcs["print_no_newline"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::printf(" ");
            std::printf("%s", args[i]->inspect().c_str());
        }
        std::fflush(stdout);
        return getNull();
    };

    // println(args...) -> same as print
    funcs["println"] = funcs["print"];

    // format(template, args...) -> formatted string
    funcs["format"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1) return makeError("format: expected at least 1 argument");
        std::string tmpl = getString(args[0]);
        std::string result;
        size_t autoIdx = 1; // args[0] is the template
        for (size_t i = 0; i < tmpl.size(); i++) {
            if (tmpl[i] == '{') {
                size_t close = tmpl.find('}', i + 1);
                if (close == std::string::npos) { result += tmpl[i]; continue; }
                std::string placeholder = tmpl.substr(i + 1, close - i - 1);
                size_t idx;
                if (placeholder.empty()) {
                    idx = autoIdx++;
                } else {
                    idx = static_cast<size_t>(std::stoll(placeholder)) + 1;
                }
                if (idx < args.size()) {
                    result += args[idx]->inspect();
                } else {
                    result += tmpl.substr(i, close - i + 1);
                }
                i = close;
            } else {
                result += tmpl[i];
            }
        }
        return newString(result);
    };

    // sprint(template, args...) -> formatted string (alias for format)
    funcs["sprint"] = funcs["format"];

    // read_line() -> string from stdin
    funcs["read_line"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        return newString(line);
    };

    // read_all() -> all of stdin as string
    funcs["read_all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::ostringstream buf;
        buf << std::cin.rdbuf();
        return newString(buf.str());
    };

    // read(prompt?) -> string with optional prompt
    funcs["read"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() > 1) return makeError("read: expected 0-1 arguments");
        if (args.size() == 1) {
            std::printf("%s", getString(args[0]).c_str());
            std::fflush(stdout);
        }
        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        return newString(line);
    };

    // read_int(prompt?) -> integer from stdin
    funcs["read_int"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() > 1) return makeError("read_int: expected 0-1 arguments");
        if (args.size() == 1) {
            std::printf("%s", getString(args[0]).c_str());
            std::fflush(stdout);
        }
        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        try { return newInteger(std::stoll(line)); }
        catch (...) { return makeError("read_int: invalid integer input"); }
    };

    // read_float(prompt?) -> float from stdin
    funcs["read_float"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() > 1) return makeError("read_float: expected 0-1 arguments");
        if (args.size() == 1) {
            std::printf("%s", getString(args[0]).c_str());
            std::fflush(stdout);
        }
        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        try { return newFloat(std::stod(line)); }
        catch (...) { return makeError("read_float: invalid float input"); }
    };

    // read_bool(prompt?) -> bool from stdin (accepts true/false/1/0/yes/no)
    funcs["read_bool"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() > 1) return makeError("read_bool: expected 0-1 arguments");
        if (args.size() == 1) {
            std::printf("%s", getString(args[0]).c_str());
            std::fflush(stdout);
        }
        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        // trim
        size_t start = line.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return makeError("read_bool: empty input");
        std::string s = line.substr(start);
        size_t end = s.find_last_not_of(" \t\n\r");
        s = s.substr(0, end + 1);
        // lowercase
        for (auto& c : s) c = std::tolower(static_cast<unsigned char>(c));
        if (s == "true" || s == "1" || s == "yes" || s == "y") return getTrue();
        if (s == "false" || s == "0" || s == "no" || s == "n") return getFalse();
        return makeError("read_bool: invalid boolean input");
    };

    // read_until(prompt?, delimiter) -> string until delimiter
    funcs["read_until"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1 || args.size() > 2) return makeError("read_until: expected 1-2 arguments");
        if (args.size() >= 2) {
            std::printf("%s", getString(args[0]).c_str());
            std::fflush(stdout);
        }
        std::string delim = (args.size() >= 2) ? getString(args[1]) : "\n";
        std::string result;
        char c;
        while (std::cin.get(c)) {
            result += c;
            if (result.size() >= delim.size() &&
                result.substr(result.size() - delim.size()) == delim) {
                return newString(result.substr(0, result.size() - delim.size()));
            }
        }
        return newString(result);
    };

    // confirm(prompt?, default?) -> bool with default
    funcs["confirm"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::string prompt = "Continue? (y/n): ";
        bool defaultVal = false;
        if (args.size() >= 1) prompt = getString(args[0]);
        if (args.size() >= 2 && args[1]) defaultVal = isTruthy(args[1]);

        std::printf("%s ", prompt.c_str());
        std::fflush(stdout);
        std::string line;
        if (!std::getline(std::cin, line)) return newBoolean(defaultVal);

        size_t start = line.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return newBoolean(defaultVal);
        std::string s = line.substr(start, 1);
        return newBoolean(s == "y" || s == "Y" || s == "1");
    };

    // choose(options_array, prompt?) -> chosen option
    funcs["choose"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1) return makeError("choose: expected at least 1 argument");
        auto opts = std::dynamic_pointer_cast<Array>(args[0]);
        if (!opts) return makeError("choose: first argument must be array");

        std::string prompt = "Choose an option";
        if (args.size() >= 2) prompt = getString(args[1]);

        // Display options
        for (size_t i = 0; i < opts->elements.size(); i++) {
            std::printf("  %zu. %s\n", i + 1, opts->elements[i]->inspect().c_str());
        }
        std::printf("%s: ", prompt.c_str());
        std::fflush(stdout);

        std::string line;
        if (!std::getline(std::cin, line)) return getNull();
        try {
            int choice = std::stoi(line);
            if (choice >= 1 && static_cast<size_t>(choice) <= opts->elements.size()) {
                return opts->elements[choice - 1];
            }
        } catch (...) {}
        return makeError("choose: invalid selection");
    };

    // progress(current, total, width?) -> progress bar string
    funcs["progress"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("progress: expected 2-3 arguments");
        int64_t current = 0, total = 0;
        if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) current = i->value;
        else return makeError("progress: current must be integer");
        if (auto i = std::dynamic_pointer_cast<Integer>(args[1])) total = i->value;
        else return makeError("progress: total must be integer");
        int width = 20;
        if (args.size() == 3) {
            if (auto i = std::dynamic_pointer_cast<Integer>(args[2])) width = static_cast<int>(i->value);
        }
        if (total <= 0) return newString("[" + std::string(width, ']'));
        double pct = std::min(1.0, static_cast<double>(current) / total);
        int filled = static_cast<int>(pct * width);
        std::string bar = "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
        char pctBuf[16];
        std::snprintf(pctBuf, sizeof(pctBuf), " %d%%", static_cast<int>(pct * 100));
        return newString(bar + pctBuf);
    };

    // spinner() -> spinning character (call repeatedly)
    static int spinnerState = 0;
    funcs["spinner"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        static const char chars[] = "|/-\\";
        return newString(std::string(1, chars[spinnerState++ % 4]));
    };

    // clear_screen() -> null
    funcs["clear_screen"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif
        return getNull();
    };

    // beep() -> null
    funcs["beep"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::printf("\a");
        std::fflush(stdout);
        return getNull();
    };

    // table(headers, rows) -> formatted table string
    funcs["table"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("table: expected 2 arguments");
        auto headers = std::dynamic_pointer_cast<Array>(args[0]);
        auto rows = std::dynamic_pointer_cast<Array>(args[1]);
        if (!headers || !rows) return makeError("table: both arguments must be arrays");

        // Find column widths
        std::vector<size_t> widths;
        for (auto& h : headers->elements) widths.push_back(h->inspect().size());
        for (auto& row : rows->elements) {
            auto r = std::dynamic_pointer_cast<Array>(row);
            if (r) {
                for (size_t i = 0; i < r->elements.size() && i < widths.size(); i++) {
                    size_t w = r->elements[i]->inspect().size();
                    if (w > widths[i]) widths[i] = w;
                }
            }
        }

        // Build table
        std::string result;
        // Header
        result += "|";
        for (size_t i = 0; i < headers->elements.size(); i++) {
            std::string h = headers->elements[i]->inspect();
            result += " " + h + std::string(widths[i] - h.size(), ' ') + " |";
        }
        result += "\n";
        // Separator
        result += "|";
        for (auto w : widths) result += std::string(w + 2, '-') + "|";
        result += "\n";
        // Rows
        for (auto& row : rows->elements) {
            auto r = std::dynamic_pointer_cast<Array>(row);
            if (!r) continue;
            result += "|";
            for (size_t i = 0; i < widths.size(); i++) {
                std::string val = (i < r->elements.size()) ? r->elements[i]->inspect() : "";
                result += " " + val + std::string(widths[i] - val.size(), ' ') + " |";
            }
            result += "\n";
        }
        return newString(result);
    };

    // json_table(data) -> formatted table from array of objects
    funcs["json_table"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("json_table: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return newString("");

        // Collect all keys
        std::vector<std::string> headers;
        for (auto& elem : arr->elements) {
            auto obj = std::dynamic_pointer_cast<Map>(elem);
            if (obj) {
                for (auto& [k, v] : obj->pairs) {
                    std::string key = k->inspect();
                    if (std::find(headers.begin(), headers.end(), key) == headers.end())
                        headers.push_back(key);
                }
            }
        }

        // Build rows
        std::vector<std::vector<std::string>> rowData;
        for (auto& elem : arr->elements) {
            auto obj = std::dynamic_pointer_cast<Map>(elem);
            if (!obj) continue;
            std::vector<std::string> row;
            for (auto& h : headers) {
                bool found = false;
                for (auto& [k, v] : obj->pairs) {
                    if (k->inspect() == h) { row.push_back(v->inspect()); found = true; break; }
                }
                if (!found) row.push_back("");
            }
            rowData.push_back(row);
        }

        // Find widths
        std::vector<size_t> widths(headers.size(), 0);
        for (auto& h : headers) if (h.size() > widths[0]) {} // placeholder
        for (size_t i = 0; i < headers.size(); i++) widths[i] = headers[i].size();
        for (auto& row : rowData)
            for (size_t i = 0; i < row.size() && i < widths.size(); i++)
                if (row[i].size() > widths[i]) widths[i] = row[i].size();

        // Build
        std::string result = "|";
        for (size_t i = 0; i < headers.size(); i++)
            result += " " + headers[i] + std::string(widths[i] - headers[i].size(), ' ') + " |";
        result += "\n|";
        for (auto w : widths) result += std::string(w + 2, '-') + "|";
        result += "\n";
        for (auto& row : rowData) {
            result += "|";
            for (size_t i = 0; i < widths.size(); i++) {
                std::string val = (i < row.size()) ? row[i] : "";
                result += " " + val + std::string(widths[i] - val.size(), ' ') + " |";
            }
            result += "\n";
        }
        return newString(result);
    };

    Registry::instance().registerModule("io", funcs);
}

} // namespace darix::native
