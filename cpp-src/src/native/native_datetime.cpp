#include "darix/native/native.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

static int64_t getInt(ObjectPtr obj) {
    if (auto i = std::dynamic_pointer_cast<Integer>(obj)) return i->value;
    if (auto f = std::dynamic_pointer_cast<Float>(obj)) return static_cast<int64_t>(f->value);
    return 0;
}

static std::tm getTM(int64_t timestamp) {
    time_t t = static_cast<time_t>(timestamp);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return tm;
}

static int64_t getTimeT(std::tm tm) {
    return static_cast<int64_t>(mktime(&tm));
}

// Format: %Y-%m-%d %H:%M:%S
static std::string formatTime(const std::tm& tm, const std::string& fmt) {
    char buf[256];
    std::strftime(buf, sizeof(buf), fmt.c_str(), &tm);
    return buf;
}

void initDatetimeModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // now() -> current timestamp (seconds since epoch)
    funcs["now"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
        return newInteger(epoch.count());
    };

    // now_ms() -> current timestamp in milliseconds
    funcs["now_ms"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return newInteger(epoch.count());
    };

    // timestamp() -> alias for now()
    funcs["timestamp"] = funcs["now"];

    // from_string(date_string) -> timestamp
    funcs["from_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_string: expected 1 argument");
        std::string s = getString(args[0]);
        std::tm tm = {};
        // Try common formats
        if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d:%d",
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            return newInteger(getTimeT(tm));
        }
        if (std::sscanf(s.c_str(), "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            return newInteger(getTimeT(tm));
        }
        return makeError("from_string: cannot parse date format (expected 'YYYY-MM-DD HH:MM:SS' or 'YYYY-MM-DD')");
    };

    // format(timestamp, format_string) -> formatted string
    funcs["format"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("format: expected 2 arguments");
        auto tm = getTM(getInt(args[0]));
        std::string fmt = getString(args[1]);
        return newString(formatTime(tm, fmt));
    };

    // year(timestamp) -> int
    funcs["year"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("year: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_year + 1900);
    };

    // month(timestamp) -> int (1-12)
    funcs["month"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("month: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_mon + 1);
    };

    // day(timestamp) -> int (1-31)
    funcs["day"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("day: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_mday);
    };

    // hour(timestamp) -> int (0-23)
    funcs["hour"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hour: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_hour);
    };

    // minute(timestamp) -> int (0-59)
    funcs["minute"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("minute: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_min);
    };

    // second(timestamp) -> int (0-59)
    funcs["second"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("second: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_sec);
    };

    // weekday(timestamp) -> int (0=Sunday, 6=Saturday)
    funcs["weekday"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("weekday: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_wday);
    };

    // day_of_year(timestamp) -> int (0-365)
    funcs["day_of_year"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("day_of_year: expected 1 argument");
        return newInteger(getTM(getInt(args[0])).tm_yday);
    };

    // is_leap_year(year) -> bool
    funcs["is_leap_year"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_leap_year: expected 1 argument");
        int64_t y = getInt(args[0]);
        return newBoolean((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0));
    };

    // days_in_month(year, month) -> int
    funcs["days_in_month"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("days_in_month: expected 2 arguments");
        int64_t y = getInt(args[0]), m = getInt(args[1]);
        int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
        if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) return newInteger(29);
        if (m >= 1 && m <= 12) return newInteger(days[m]);
        return makeError("days_in_month: month must be 1-12");
    };

    // add_days(timestamp, days) -> timestamp
    funcs["add_days"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_days: expected 2 arguments");
        return newInteger(getInt(args[0]) + getInt(args[1]) * 86400);
    };

    // add_hours(timestamp, hours) -> timestamp
    funcs["add_hours"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_hours: expected 2 arguments");
        return newInteger(getInt(args[0]) + getInt(args[1]) * 3600);
    };

    // add_minutes(timestamp, minutes) -> timestamp
    funcs["add_minutes"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_minutes: expected 2 arguments");
        return newInteger(getInt(args[0]) + getInt(args[1]) * 60);
    };

    // add_seconds(timestamp, seconds) -> timestamp
    funcs["add_seconds"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_seconds: expected 2 arguments");
        return newInteger(getInt(args[0]) + getInt(args[1]));
    };

    // diff(timestamp1, timestamp2) -> seconds difference
    funcs["diff"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("diff: expected 2 arguments");
        return newInteger(getInt(args[0]) - getInt(args[1]));
    };

    // diff_days(timestamp1, timestamp2) -> days difference
    funcs["diff_days"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("diff_days: expected 2 arguments");
        return newInteger((getInt(args[0]) - getInt(args[1])) / 86400);
    };

    // to_string(timestamp) -> "YYYY-MM-DD HH:MM:SS"
    funcs["to_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_string: expected 1 argument");
        auto tm = getTM(getInt(args[0]));
        return newString(formatTime(tm, "%Y-%m-%d %H:%M:%S"));
    };

    // to_date_string(timestamp) -> "YYYY-MM-DD"
    funcs["to_date_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_date_string: expected 1 argument");
        auto tm = getTM(getInt(args[0]));
        return newString(formatTime(tm, "%Y-%m-%d"));
    };

    // to_time_string(timestamp) -> "HH:MM:SS"
    funcs["to_time_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_time_string: expected 1 argument");
        auto tm = getTM(getInt(args[0]));
        return newString(formatTime(tm, "%H:%M:%S"));
    };

    // to_iso(timestamp) -> ISO 8601 string
    funcs["to_iso"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_iso: expected 1 argument");
        auto tm = getTM(getInt(args[0]));
        return newString(formatTime(tm, "%Y-%m-%dT%H:%M:%S"));
    };

    // make(year, month, day, hour?, minute?, second?) -> timestamp
    funcs["make"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 3 || args.size() > 6) return makeError("make: expected 3-6 arguments");
        std::tm tm = {};
        tm.tm_year = static_cast<int>(getInt(args[0])) - 1900;
        tm.tm_mon = static_cast<int>(getInt(args[1])) - 1;
        tm.tm_mday = static_cast<int>(getInt(args[2]));
        if (args.size() >= 4) tm.tm_hour = static_cast<int>(getInt(args[3]));
        if (args.size() >= 5) tm.tm_min = static_cast<int>(getInt(args[4]));
        if (args.size() >= 6) tm.tm_sec = static_cast<int>(getInt(args[5]));
        return newInteger(getTimeT(tm));
    };

    // sleep(seconds)
    funcs["sleep"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sleep: expected 1 argument");
        int64_t ms = getInt(args[0]) * 1000;
        if (args[0]->type() == ObjectType::FLOAT) {
            ms = static_cast<int64_t>(std::dynamic_pointer_cast<Float>(args[0])->value * 1000);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return getNull();
    };

    // sleep_ms(milliseconds)
    funcs["sleep_ms"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sleep_ms: expected 1 argument");
        std::this_thread::sleep_for(std::chrono::milliseconds(getInt(args[0])));
        return getNull();
    };

    // parse(format, string) -> timestamp
    funcs["parse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("parse: expected 2 arguments");
        std::string fmt = getString(args[0]);
        std::string s = getString(args[1]);
        std::tm tm = {};
        // Try parsing with strptime-like logic (simplified)
        if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d:%d",
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
            tm.tm_year -= 1900; tm.tm_mon -= 1;
            return newInteger(getTimeT(tm));
        }
        if (std::sscanf(s.c_str(), "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3) {
            tm.tm_year -= 1900; tm.tm_mon -= 1;
            return newInteger(getTimeT(tm));
        }
        return makeError("parse: cannot parse date string");
    };

    // is_before(t1, t2) -> bool
    funcs["is_before"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_before: expected 2 arguments");
        return newBoolean(getInt(args[0]) < getInt(args[1]));
    };

    // is_after(t1, t2) -> bool
    funcs["is_after"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_after: expected 2 arguments");
        return newBoolean(getInt(args[0]) > getInt(args[1]));
    };

    // is_same_day(t1, t2) -> bool
    funcs["is_same_day"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_same_day: expected 2 arguments");
        auto a = getTM(getInt(args[0])), b = getTM(getInt(args[1]));
        return newBoolean(a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday == b.tm_mday);
    };

    // timezone_offset() -> seconds offset from UTC
    funcs["timezone_offset"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto now = std::chrono::system_clock::now();
        auto local = std::chrono::system_clock::to_time_t(now);
        std::tm localTm, utcTm;
#ifdef _WIN32
        localtime_s(&localTm, &local);
        gmtime_s(&utcTm, &local);
#else
        localtime_r(&local, &localTm);
        gmtime_r(&local, &utcTm);
#endif
        return newInteger(static_cast<int64_t>(mktime(&localTm) - mktime(&utcTm)));
    };

    // clock() -> high-res time in milliseconds
    funcs["clock"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return newInteger(ms.count());
    };

    Registry::instance().registerModule("datetime", funcs);
}

} // namespace darix::native
