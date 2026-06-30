#include "darix/native/native.hpp"
#include <cstdlib>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

void initOsModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // getenv(name) -> string or null
    funcs["getenv"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("getenv: expected 1 argument");
        const char* val = std::getenv(getString(args[0]).c_str());
        return val ? newString(val) : getNull();
    };

    // setenv(name, value) -> bool
    funcs["setenv"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("setenv: expected 2 arguments");
#ifdef _WIN32
        return newBoolean(_putenv_s(getString(args[0]).c_str(), getString(args[1]).c_str()) == 0);
#else
        return newBoolean(setenv(getString(args[0]).c_str(), getString(args[1]).c_str(), 1) == 0);
#endif
    };

    // unsetenv(name) -> bool
    funcs["unsetenv"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unsetenv: expected 1 argument");
#ifdef _WIN32
        return newBoolean(_putenv_s(getString(args[0]).c_str(), "") == 0);
#else
        return newBoolean(unsetenv(getString(args[0]).c_str()) == 0);
#endif
    };

    // platform() -> "windows", "linux", "darwin"
    funcs["platform"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        return newString("windows");
#elif defined(__APPLE__)
        return newString("darwin");
#else
        return newString("linux");
#endif
    };

    // arch() -> "x86_64", "aarch64", etc.
    funcs["arch"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
        return newString("x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
        return newString("aarch64");
#elif defined(__i386__) || defined(_M_IX86)
        return newString("x86");
#else
        return newString("unknown");
#endif
    };

    // hostname() -> string
    funcs["hostname"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        char name[256];
        DWORD size = sizeof(name);
        if (GetComputerNameA(name, &size)) return newString(name);
        return newString("unknown");
#else
        char name[256];
        if (gethostname(name, sizeof(name)) == 0) return newString(name);
        return newString("unknown");
#endif
    };

    // getpid() -> int
    funcs["getpid"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        return newInteger(static_cast<int64_t>(GetCurrentProcessId()));
#else
        return newInteger(static_cast<int64_t>(getpid()));
#endif
    };

    // exit(code) -> never returns
    funcs["exit"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        int code = 0;
        if (!args.empty()) {
            if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) code = static_cast<int>(i->value);
        }
        std::exit(code);
        return getNull(); // unreachable
    };

    // exec(command) -> map {exit_code, stdout}
    funcs["exec"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("exec: expected 1 argument");
        std::string cmd = getString(args[0]);
#ifdef _WIN32
        cmd += " 2>&1";
        FILE* pipe = _popen(cmd.c_str(), "r");
        if (!pipe) return makeError("exec: failed to run command");
        std::string output;
        char buf[4096];
        while (fgets(buf, sizeof(buf), pipe)) output += buf;
        int status = _pclose(pipe);
        auto result = std::make_shared<Map>();
        auto k1 = newString("exit_code"); auto v1 = newInteger(static_cast<int64_t>(status));
        auto k2 = newString("stdout"); auto v2 = newString(output);
        result->pairs.emplace_back(k1, v1);
        result->pairs.emplace_back(k2, v2);
        return result;
#else
        cmd += " 2>&1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return makeError("exec: failed to run command");
        std::string output;
        char buf[4096];
        while (fgets(buf, sizeof(buf), pipe)) output += buf;
        int status = pclose(pipe);
        auto result = std::make_shared<Map>();
        auto k1 = newString("exit_code"); auto v1 = newInteger(static_cast<int64_t>(status));
        auto k2 = newString("stdout"); auto v2 = newString(output);
        result->pairs.emplace_back(k1, v1);
        result->pairs.emplace_back(k2, v2);
        return result;
#endif
    };

    // sleep(seconds)
    funcs["sleep"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sleep: expected 1 argument");
        if (args[0]->type() == ObjectType::FLOAT) {
            auto ms = static_cast<int64_t>(std::dynamic_pointer_cast<Float>(args[0])->value * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        } else if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) {
            std::this_thread::sleep_for(std::chrono::seconds(i->value));
        }
        return getNull();
    };

    // cpu_count() -> int
    funcs["cpu_count"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newInteger(static_cast<int64_t>(std::thread::hardware_concurrency()));
    };

    // uname() -> map with platform info
    funcs["uname"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto result = std::make_shared<Map>();
#ifdef _WIN32
        OSVERSIONINFOEXA osvi{};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        result->pairs.push_back({newString("system"), newString("Windows")});
        result->pairs.push_back({newString("release"), newString("unknown")});
        result->pairs.push_back({newString("machine"), newString("x86_64")});
#else
        struct utsname u;
        if (uname(&u) == 0) {
            result->pairs.push_back({newString("system"), newString(u.sysname)});
            result->pairs.push_back({newString("node"), newString(u.nodename)});
            result->pairs.push_back({newString("release"), newString(u.release)});
            result->pairs.push_back({newString("version"), newString(u.version)});
            result->pairs.push_back({newString("machine"), newString(u.machine)});
        }
#endif
        return result;
    };

    // clock() -> high-resolution time in milliseconds
    funcs["clock"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return newInteger(ms.count());
    };

    // memory_info() -> map with memory info (simplified)
    funcs["memory_info"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto result = std::make_shared<Map>();
#ifdef _WIN32
        MEMORYSTATUSEX stat{};
        stat.dwLength = sizeof(stat);
        if (GlobalMemoryStatusEx(&stat)) {
            result->pairs.push_back({newString("total"), newInteger(static_cast<int64_t>(stat.ullTotalPhys))});
            result->pairs.push_back({newString("free"), newInteger(static_cast<int64_t>(stat.ullAvailPhys))});
            result->pairs.push_back({newString("used"), newInteger(static_cast<int64_t>(stat.ullTotalPhys - stat.ullAvailPhys))});
            result->pairs.push_back({newString("usage_percent"), newInteger(static_cast<int64_t>(stat.dwMemoryLoad))});
        }
#else
        long pages = sysconf(_SC_PHYS_PAGES);
        long avail = sysconf(_SC_AVPHYS_PAGES);
        long pageSize = sysconf(_SC_PAGE_SIZE);
        result->pairs.push_back({newString("total"), newInteger(pages * pageSize)});
        result->pairs.push_back({newString("free"), newInteger(avail * pageSize)});
        result->pairs.push_back({newString("used"), newInteger((pages - avail) * pageSize)});
        result->pairs.push_back({newString("usage_percent"), newInteger(pages > 0 ? ((pages - avail) * 100 / pages) : 0)});
#endif
        return result;
    };

    // user() -> username string
    funcs["user"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        char name[256];
        DWORD size = sizeof(name);
        if (GetUserNameA(name, &size)) return newString(name);
        return newString("unknown");
#else
        const char* name = getenv("USER");
        if (!name) name = getenv("LOGNAME");
        return newString(name ? name : "unknown");
#endif
    };

    // home() -> home directory
    funcs["home"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
#ifdef _WIN32
        const char* home = getenv("USERPROFILE");
        return newString(home ? home : "C:\\");
#else
        const char* home = getenv("HOME");
        return newString(home ? home : "/");
#endif
    };

    Registry::instance().registerModule("os", funcs);
}

} // namespace darix::native
