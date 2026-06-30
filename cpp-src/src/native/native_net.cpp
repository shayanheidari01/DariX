#include "darix/native/native.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using sock_t = SOCKET;
#define CLOSE_SOCKET closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
using sock_t = int;
#define CLOSE_SOCKET close
#endif

#include <cstring>
#include <thread>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static std::string getString(ObjectPtr obj) {
    if (auto s = std::dynamic_pointer_cast<String>(obj)) return s->value;
    return "";
}

#ifdef _WIN32
static bool winsockInit = false;
static void ensureWinsock() {
    if (!winsockInit) { WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa); winsockInit = true; }
}
#endif

void initNetModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // tcp_connect(host, port) -> socket_fd (as integer)
    funcs["tcp_connect"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("tcp_connect: expected 2 arguments");
        std::string host = getString(args[0]);
        auto portObj = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!portObj) return makeError("tcp_connect: port must be integer");

#ifdef _WIN32
        ensureWinsock();
#endif
        sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
        if (
#ifdef _WIN32
            fd == INVALID_SOCKET
#else
            fd < 0
#endif
        ) return makeError("tcp_connect: socket creation failed");

        struct addrinfo hints{}, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        std::string portStr = std::to_string(portObj->value);

        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
            CLOSE_SOCKET(fd);
            return makeError("tcp_connect: cannot resolve host");
        }

        int rc = ::connect(fd, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
        if (rc != 0) {
            CLOSE_SOCKET(fd);
            return makeError("tcp_connect: connection failed");
        }
        return newInteger(static_cast<int64_t>(fd));
    };

    // tcp_send(fd, data) -> bytes sent
    funcs["tcp_send"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("tcp_send: expected 2 arguments");
        auto fdObj = std::dynamic_pointer_cast<Integer>(args[0]);
        if (!fdObj) return makeError("tcp_send: fd must be integer");
        std::string data = getString(args[1]);
        sock_t fd = static_cast<sock_t>(fdObj->value);
        auto sent = ::send(fd, data.c_str(), static_cast<int>(data.size()), 0);
        if (sent < 0) return makeError("tcp_send: send failed");
        return newInteger(static_cast<int64_t>(sent));
    };

    // tcp_recv(fd, bufsize) -> string
    funcs["tcp_recv"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("tcp_recv: expected 2 arguments");
        auto fdObj = std::dynamic_pointer_cast<Integer>(args[0]);
        auto szObj = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!fdObj || !szObj) return makeError("tcp_recv: arguments must be integers");
        sock_t fd = static_cast<sock_t>(fdObj->value);
        int bufsize = static_cast<int>(szObj->value);
        if (bufsize <= 0 || bufsize > 65536) bufsize = 4096;
        std::vector<char> buf(bufsize);
        auto received = ::recv(fd, buf.data(), bufsize, 0);
        if (received <= 0) return newString("");
        return newString(std::string(buf.data(), received));
    };

    // tcp_close(fd) -> bool
    funcs["tcp_close"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("tcp_close: expected 1 argument");
        auto fdObj = std::dynamic_pointer_cast<Integer>(args[0]);
        if (!fdObj) return makeError("tcp_close: fd must be integer");
        return newBoolean(CLOSE_SOCKET(static_cast<sock_t>(fdObj->value)) == 0);
    };

    // udp_send(host, port, data) -> bytes sent
    funcs["udp_send"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("udp_send: expected 3 arguments");
        std::string host = getString(args[0]);
        auto portObj = std::dynamic_pointer_cast<Integer>(args[1]);
        std::string data = getString(args[2]);
        if (!portObj) return makeError("udp_send: port must be integer");

#ifdef _WIN32
        ensureWinsock();
#endif
        sock_t fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (
#ifdef _WIN32
            fd == INVALID_SOCKET
#else
            fd < 0
#endif
        ) return makeError("udp_send: socket creation failed");

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(portObj->value));
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        auto sent = ::sendto(fd, data.c_str(), static_cast<int>(data.size()), 0,
                             reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
        CLOSE_SOCKET(fd);
        if (sent < 0) return makeError("udp_send: send failed");
        return newInteger(static_cast<int64_t>(sent));
    };

    // http_get(url) -> {status, body}
    funcs["http_get"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("http_get: expected 1 argument");
        std::string url = getString(args[0]);

        // Simple HTTP GET - parse host and path
        std::string host, path;
        if (url.find("http://") == 0) {
            auto rest = url.substr(7);
            auto slashPos = rest.find('/');
            host = (slashPos != std::string::npos) ? rest.substr(0, slashPos) : rest;
            path = (slashPos != std::string::npos) ? rest.substr(slashPos) : "/";
        } else {
            return makeError("http_get: only http:// URLs supported");
        }

#ifdef _WIN32
        ensureWinsock();
#endif
        sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
        if (
#ifdef _WIN32
            fd == INVALID_SOCKET
#else
            fd < 0
#endif
        ) return makeError("http_get: socket creation failed");

        struct addrinfo hints{}, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        // Extract port (default 80)
        int port = 80;
        auto colonPos = host.find(':');
        if (colonPos != std::string::npos) {
            port = std::stoi(host.substr(colonPos + 1));
            host = host.substr(0, colonPos);
        }

        std::string portStr = std::to_string(port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
            CLOSE_SOCKET(fd);
            return makeError("http_get: cannot resolve host");
        }

        if (::connect(fd, result->ai_addr, result->ai_addrlen) != 0) {
            freeaddrinfo(result);
            CLOSE_SOCKET(fd);
            return makeError("http_get: connection failed");
        }
        freeaddrinfo(result);

        // Send HTTP request
        std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
        ::send(fd, req.c_str(), static_cast<int>(req.size()), 0);

        // Read response
        std::string response;
        char buf[4096];
        int n;
        while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
            response.append(buf, n);
        }
        CLOSE_SOCKET(fd);

        // Parse status code
        int status = 0;
        auto firstLine = response.find("\r\n");
        if (firstLine != std::string::npos) {
            auto statusStart = response.find(' ');
            if (statusStart != std::string::npos && statusStart < firstLine) {
                status = std::stoi(response.substr(statusStart + 1, 3));
            }
        }

        // Extract body (after double CRLF)
        auto bodyStart = response.find("\r\n\r\n");
        std::string body = (bodyStart != std::string::npos) ? response.substr(bodyStart + 4) : "";

        auto responseMap = std::make_shared<Map>();
        responseMap->pairs.push_back({newString("status"), newInteger(status)});
        responseMap->pairs.push_back({newString("body"), newString(body)});
        return responseMap;
    };

    // http_post(url, body, content_type?) -> {status, body}
    funcs["http_post"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2) return makeError("http_post: expected 2-3 arguments");
        std::string url = getString(args[0]);
        std::string body = getString(args[1]);
        std::string contentType = "text/plain";
        if (args.size() >= 3 && args[2] && args[2]->type() == ObjectType::STRING)
            contentType = getString(args[2]);

        std::string host, path;
        if (url.find("http://") == 0) {
            auto rest = url.substr(7);
            auto slashPos = rest.find('/');
            host = (slashPos != std::string::npos) ? rest.substr(0, slashPos) : rest;
            path = (slashPos != std::string::npos) ? rest.substr(slashPos) : "/";
        } else {
            return makeError("http_post: only http:// URLs supported");
        }

#ifdef _WIN32
        ensureWinsock();
#endif
        sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
        if (
#ifdef _WIN32
            fd == INVALID_SOCKET
#else
            fd < 0
#endif
        ) return makeError("http_post: socket creation failed");

        struct addrinfo hints{}, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        int port = 80;
        auto colonPos = host.find(':');
        if (colonPos != std::string::npos) {
            port = std::stoi(host.substr(colonPos + 1));
            host = host.substr(0, colonPos);
        }
        std::string portStr = std::to_string(port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
            CLOSE_SOCKET(fd);
            return makeError("http_post: cannot resolve host");
        }
        if (::connect(fd, result->ai_addr, result->ai_addrlen) != 0) {
            freeaddrinfo(result);
            CLOSE_SOCKET(fd);
            return makeError("http_post: connection failed");
        }
        freeaddrinfo(result);

        std::string req = "POST " + path + " HTTP/1.1\r\nHost: " + host +
            "\r\nContent-Type: " + contentType +
            "\r\nContent-Length: " + std::to_string(body.size()) +
            "\r\nConnection: close\r\n\r\n" + body;
        ::send(fd, req.c_str(), static_cast<int>(req.size()), 0);

        std::string response;
        char buf[4096];
        int n;
        while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) response.append(buf, n);
        CLOSE_SOCKET(fd);

        int status = 0;
        auto firstLine = response.find("\r\n");
        if (firstLine != std::string::npos) {
            auto s = response.find(' ');
            if (s != std::string::npos && s < firstLine) status = std::stoi(response.substr(s + 1, 3));
        }
        auto bodyStart = response.find("\r\n\r\n");
        std::string respBody = (bodyStart != std::string::npos) ? response.substr(bodyStart + 4) : "";

        auto res = std::make_shared<Map>();
        res->pairs.push_back({newString("status"), newInteger(status)});
        res->pairs.push_back({newString("body"), newString(respBody)});
        return res;
    };

    // resolve(host) -> array of IP strings
    funcs["resolve"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("resolve: expected 1 argument");
#ifdef _WIN32
        ensureWinsock();
#endif
        std::string host = getString(args[0]);
        struct addrinfo hints{}, *result;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0)
            return makeError("resolve: cannot resolve host");
        std::vector<ObjectPtr> ips;
        for (auto p = result; p; p = p->ai_next) {
            char ip[INET_ADDRSTRLEN];
            auto addr = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            ips.push_back(newString(ip));
        }
        freeaddrinfo(result);
        return newArray(ips);
    };

    Registry::instance().registerModule("net", funcs);
}

} // namespace darix::native
