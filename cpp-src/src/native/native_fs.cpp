#include "darix/native/native.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

void initFsModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // read(path) -> string
    funcs["read"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("read: expected 1 argument");
        std::string path = getString(args[0]);
        std::ifstream file(path);
        if (!file.is_open()) return makeError("read: cannot open file '" + path + "'");
        std::stringstream buffer;
        buffer << file.rdbuf();
        return newString(buffer.str());
    };

    // write(path, content) -> bool
    funcs["write"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("write: expected 2 arguments");
        std::string path = getString(args[0]);
        std::string content = getString(args[1]);
        std::ofstream file(path);
        if (!file.is_open()) return makeError("write: cannot open file '" + path + "'");
        file << content;
        return newBoolean(file.good());
    };

    // append(path, content) -> bool
    funcs["append"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("append: expected 2 arguments");
        std::string path = getString(args[0]);
        std::string content = getString(args[1]);
        std::ofstream file(path, std::ios::app);
        if (!file.is_open()) return makeError("append: cannot open file '" + path + "'");
        file << content;
        return newBoolean(file.good());
    };

    // exists(path) -> bool
    funcs["exists"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("exists: expected 1 argument");
        return newBoolean(fs::exists(getString(args[0])));
    };

    // is_file(path) -> bool
    funcs["is_file"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_file: expected 1 argument");
        return newBoolean(fs::is_regular_file(getString(args[0])));
    };

    // is_dir(path) -> bool
    funcs["is_dir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_dir: expected 1 argument");
        return newBoolean(fs::is_directory(getString(args[0])));
    };

    // mkdir(path) -> bool
    funcs["mkdir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("mkdir: expected 1 argument");
        std::error_code ec;
        bool ok = fs::create_directories(getString(args[0]), ec);
        return newBoolean(ok && !ec);
    };

    // rmdir(path) -> bool
    funcs["rmdir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("rmdir: expected 1 argument");
        std::error_code ec;
        bool ok = fs::remove_all(getString(args[0]), ec);
        return newBoolean(ok && !ec);
    };

    // remove(path) -> bool
    funcs["remove"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("remove: expected 1 argument");
        std::error_code ec;
        bool ok = fs::remove(getString(args[0]), ec);
        return newBoolean(ok && !ec);
    };

    // rename(old, new) -> bool
    funcs["rename"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("rename: expected 2 arguments");
        std::error_code ec;
        fs::rename(getString(args[0]), getString(args[1]), ec);
        return newBoolean(!ec);
    };

    // copy(src, dst) -> bool
    funcs["copy"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("copy: expected 2 arguments");
        std::error_code ec;
        bool ok = fs::copy_file(getString(args[0]), getString(args[1]), fs::copy_options::overwrite_existing, ec);
        return newBoolean(ok && !ec);
    };

    // size(path) -> int (file size in bytes)
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        std::error_code ec;
        auto sz = fs::file_size(getString(args[0]), ec);
        if (ec) return makeError("size: cannot get file size");
        return newInteger(static_cast<int64_t>(sz));
    };

    // list_dir(path) -> array of directory entries
    funcs["list_dir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("list_dir: expected 1 argument");
        std::string path = getString(args[0]);
        std::error_code ec;
        std::vector<ObjectPtr> result;
        for (auto& entry : fs::directory_iterator(path, ec)) {
            result.push_back(newString(entry.path().filename().string()));
        }
        if (ec) return makeError("list_dir: cannot read directory");
        return newArray(result);
    };

    // list_dir_full(path) -> array of {name, is_dir, size} objects
    funcs["list_dir_full"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("list_dir_full: expected 1 argument");
        std::string path = getString(args[0]);
        std::error_code ec;
        std::vector<ObjectPtr> result;
        for (auto& entry : fs::directory_iterator(path, ec)) {
            auto info = std::make_shared<Map>();
            info->pairs.push_back({newString("name"), newString(entry.path().filename().string())});
            info->pairs.push_back({newString("is_dir"), newBoolean(entry.is_directory())});
            auto sz = entry.file_size(ec);
            info->pairs.push_back({newString("size"), newInteger(ec ? 0 : static_cast<int64_t>(sz))});
            result.push_back(info);
        }
        if (ec) return makeError("list_dir_full: cannot read directory");
        return newArray(result);
    };

    // cwd() -> current working directory
    funcs["cwd"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newString(fs::current_path().string());
    };

    // chdir(path) -> bool
    funcs["chdir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("chdir: expected 1 argument");
        std::error_code ec;
        fs::current_path(getString(args[0]), ec);
        return newBoolean(!ec);
    };

    // join(paths...) -> joined path
    funcs["join"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2) return makeError("join: expected at least 2 arguments");
        fs::path result = getString(args[0]);
        for (size_t i = 1; i < args.size(); i++) {
            result /= getString(args[i]);
        }
        return newString(result.string());
    };

    // parent(path) -> parent directory
    funcs["parent"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("parent: expected 1 argument");
        return newString(fs::path(getString(args[0])).parent_path().string());
    };

    // filename(path) -> filename
    funcs["filename"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("filename: expected 1 argument");
        return newString(fs::path(getString(args[0])).filename().string());
    };

    // extension(path) -> extension
    funcs["extension"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("extension: expected 1 argument");
        return newString(fs::path(getString(args[0])).extension().string());
    };

    // stem(path) -> filename without extension
    funcs["stem"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("stem: expected 1 argument");
        return newString(fs::path(getString(args[0])).stem().string());
    };

    // absolute(path) -> absolute path
    funcs["absolute"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("absolute: expected 1 argument");
        std::error_code ec;
        return newString(fs::absolute(getString(args[0]), ec).string());
    };

    // temp_dir() -> temp directory path
    funcs["temp_dir"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newString(fs::temp_directory_path().string());
    };

    // env(name) -> env variable value or null
    funcs["env"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("env: expected 1 argument");
        const char* val = std::getenv(getString(args[0]).c_str());
        return val ? newString(val) : getNull();
    };

    Registry::instance().registerModule("fs", funcs);
}

} // namespace darix::native
