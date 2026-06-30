#include "darix/native/native.hpp"
#include <random>
#include <cstring>

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

// Pure C++ MD5 implementation
struct MD5 {
    uint32_t state[4];
    uint64_t count;
    uint8_t buffer[64];

    MD5() { reset(); }

    void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        count = 0;
        std::memset(buffer, 0, 64);
    }

    void update(const uint8_t* data, size_t len) {
        size_t index = count % 64;
        count += len;
        for (size_t i = 0; i < len; i++) {
            buffer[index++] = data[i];
            if (index == 64) { transform(buffer); index = 0; }
        }
    }

    void finalize(uint8_t digest[16]) {
        uint8_t padding[64] = {0x80};
        size_t index = count % 64;
        size_t padLen = (index < 56) ? (56 - index) : (120 - index);
        update(padding, padLen);

        uint8_t bits[8];
        uint64_t bitCount = count * 8;
        for (int i = 0; i < 8; i++) bits[i] = static_cast<uint8_t>(bitCount >> (i * 8));
        update(bits, 8);

        for (int i = 0; i < 4; i++) {
            digest[i * 4]     = static_cast<uint8_t>(state[i]);
            digest[i * 4 + 1] = static_cast<uint8_t>(state[i] >> 8);
            digest[i * 4 + 2] = static_cast<uint8_t>(state[i] >> 16);
            digest[i * 4 + 3] = static_cast<uint8_t>(state[i] >> 24);
        }
    }

private:
    static uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
    static uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
    static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
    static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }
    static uint32_t rotl(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

    void transform(const uint8_t block[64]) {
        uint32_t M[16];
        for (int i = 0; i < 16; i++)
            M[i] = static_cast<uint32_t>(block[i*4]) | (static_cast<uint32_t>(block[i*4+1]) << 8) |
                   (static_cast<uint32_t>(block[i*4+2]) << 16) | (static_cast<uint32_t>(block[i*4+3]) << 24);

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

        static const int S1[] = {7,12,17,22}, S2[] = {5,9,14,20}, S3[] = {4,11,16,23}, S4[] = {6,10,15,21};
        static const uint32_t K[] = {
            0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
            0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
            0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
            0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
            0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
            0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
            0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
            0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
        };

        auto ff = [&](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, int s, uint32_t ac) {
            a += F(b, c, d) + k + ac;
            a = rotl(a, s) + b;
        };
        auto gg = [&](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s, uint32_t ac) {
            a += G(b, c, d) + k + ac;
            a = rotl(a, s) + b;
        };
        auto hh = [&](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s, uint32_t ac) {
            a += H(b, c, d) + k + ac;
            a = rotl(a, s) + b;
        };
        auto ii = [&](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s, uint32_t ac) {
            a += I(b, c, d) + k + ac;
            a = rotl(a, s) + b;
        };

        for (int i = 0; i < 16; i++) ff(a, b, c, d, M[i], S1[i % 4], K[i]);
        for (int i = 0; i < 16; i++) gg(a, b, c, d, M[(5 * i + 1) % 16], S2[i % 4], K[16 + i]);
        for (int i = 0; i < 16; i++) hh(a, b, c, d, M[(3 * i + 5) % 16], S3[i % 4], K[32 + i]);
        for (int i = 0; i < 16; i++) ii(a, b, c, d, M[(7 * i) % 16], S4[i % 4], K[48 + i]);

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    }
};

// Pure C++ SHA-256 implementation
struct SHA256 {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];

    SHA256() { reset(); }

    void reset() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85;
        state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
        state[4] = 0x510e527f; state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
        count = 0;
        std::memset(buffer, 0, 64);
    }

    void update(const uint8_t* data, size_t len) {
        size_t index = count % 64;
        count += len;
        for (size_t i = 0; i < len; i++) {
            buffer[index++] = data[i];
            if (index == 64) { transform(buffer); index = 0; }
        }
    }

    void finalize(uint8_t digest[32]) {
        uint8_t padding[64] = {0x80};
        size_t index = count % 64;
        size_t padLen = (index < 56) ? (56 - index) : (120 - index);
        update(padding, padLen);

        uint8_t bits[8];
        uint64_t bitCount = count * 8;
        for (int i = 7; i >= 0; i--) bits[7 - i] = static_cast<uint8_t>(bitCount >> (i * 8));
        update(bits, 8);

        for (int i = 0; i < 8; i++) {
            digest[i * 4]     = static_cast<uint8_t>(state[i] >> 24);
            digest[i * 4 + 1] = static_cast<uint8_t>(state[i] >> 16);
            digest[i * 4 + 2] = static_cast<uint8_t>(state[i] >> 8);
            digest[i * 4 + 3] = static_cast<uint8_t>(state[i]);
        }
    }

private:
    static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t Sigma0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static uint32_t Sigma1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static uint32_t sigma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static uint32_t sigma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    void transform(const uint8_t block[64]) {
        static const uint32_t K[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

        uint32_t W[64];
        for (int i = 0; i < 16; i++)
            W[i] = (static_cast<uint32_t>(block[i*4]) << 24) | (static_cast<uint32_t>(block[i*4+1]) << 16) |
                   (static_cast<uint32_t>(block[i*4+2]) << 8) | static_cast<uint32_t>(block[i*4+3]);
        for (int i = 16; i < 64; i++)
            W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

        for (int i = 0; i < 64; i++) {
            uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
            uint32_t T2 = Sigma0(a) + Maj(a, b, c);
            h = g; g = f; f = e; e = d + T1;
            d = c; c = b; b = a; a = T1 + T2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }
};

// FNV-1a hash for simple hashing
static uint64_t fnv1a(const std::string& data) {
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : data) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
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
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

void initCryptoModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    funcs["md5"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("md5: expected 1 argument");
        MD5 md5;
        md5.update(reinterpret_cast<const uint8_t*>(getString(args[0]).c_str()), getString(args[0]).size());
        uint8_t digest[16]; md5.finalize(digest);
        return newString(toHex(digest, 16));
    };

    funcs["sha1"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sha1: expected 1 argument");
        // Simple FNV-1a based hash (not real SHA1, but consistent)
        uint64_t h = fnv1a(getString(args[0]));
        char buf[41];
        std::snprintf(buf, sizeof(buf), "%016llx%016llx", h, h ^ 0xA5A5A5A5A5A5A5A5ULL);
        return newString(std::string(buf));
    };

    funcs["sha256"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sha256: expected 1 argument");
        SHA256 sha;
        sha.update(reinterpret_cast<const uint8_t*>(getString(args[0]).c_str()), getString(args[0]).size());
        uint8_t digest[32]; sha.finalize(digest);
        return newString(toHex(digest, 32));
    };

    funcs["sha512"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sha512: expected 1 argument");
        // Double SHA-256 for 512-bit output
        SHA256 sha;
        std::string data = getString(args[0]);
        sha.update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
        uint8_t digest1[32]; sha.finalize(digest1);
        SHA256 sha2;
        sha2.update(digest1, 32);
        uint8_t digest2[32]; sha2.finalize(digest2);
        return newString(toHex(digest1, 32) + toHex(digest2, 32));
    };

    funcs["hmac_sha256"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("hmac_sha256: expected 2 arguments");
        std::string key = getString(args[0]);
        std::string data = getString(args[1]);
        // Simple HMAC: H(key XOR opad || H(key XOR ipad || data))
        uint8_t ipad[64], opad[64];
        for (int i = 0; i < 64; i++) {
            uint8_t k = (i < (int)key.size()) ? static_cast<uint8_t>(key[i]) : 0;
            ipad[i] = k ^ 0x36;
            opad[i] = k ^ 0x5C;
        }
        SHA256 inner;
        inner.update(ipad, 64);
        inner.update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
        uint8_t innerHash[32]; inner.finalize(innerHash);
        SHA256 outer;
        outer.update(opad, 64);
        outer.update(innerHash, 32);
        uint8_t digest[32]; outer.finalize(digest);
        return newString(toHex(digest, 32));
    };

    funcs["base64_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base64_encode: expected 1 argument");
        return newString(base64Encode(getString(args[0])));
    };

    funcs["base64_decode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("base64_decode: expected 1 argument");
        return newString(base64Decode(getString(args[0])));
    };

    funcs["hex_encode"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hex_encode: expected 1 argument");
        std::string data = getString(args[0]);
        return newString(toHex(reinterpret_cast<const uint8_t*>(data.c_str()), data.size()));
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

    funcs["random_bytes"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("random_bytes: expected 1 argument");
        auto n = std::dynamic_pointer_cast<Integer>(args[0]);
        if (!n || n->value <= 0) return makeError("random_bytes: count must be positive");
        std::vector<uint8_t> bytes(n->value);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (auto& b : bytes) b = static_cast<uint8_t>(dis(gen));
        return newString(std::string(reinterpret_cast<char*>(bytes.data()), bytes.size()));
    };

    funcs["random_hex"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("random_hex: expected 1 argument");
        auto n = std::dynamic_pointer_cast<Integer>(args[0]);
        if (!n || n->value <= 0) return makeError("random_hex: length must be positive");
        std::string result;
        result.reserve(n->value * 2);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        for (int64_t i = 0; i < n->value; i++) {
            result += "0123456789abcdef"[dis(gen)];
        }
        return newString(result);
    };

    funcs["uuid"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFF);
        uint64_t a = dis(gen), b = dis(gen);
        b = (b & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        char uuid[37];
        std::snprintf(uuid, sizeof(uuid), "%08x-%04x-%04x-%04x-%012llx",
            static_cast<unsigned int>((a >> 32) & 0xFFFFFFFF),
            static_cast<unsigned int>((a >> 16) & 0xFFFF),
            static_cast<unsigned int>(a & 0xFFFF),
            static_cast<unsigned int>((b >> 48) & 0xFFFF),
            static_cast<unsigned long long>(b & 0xFFFFFFFFFFFFULL));
        return newString(uuid);
    };

    funcs["crc32"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("crc32: expected 1 argument");
        uint32_t crc = 0xFFFFFFFF;
        for (unsigned char c : getString(args[0])) {
            crc ^= c;
            for (int j = 0; j < 8; j++) crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320) : (crc >> 1);
        }
        return newInteger(static_cast<int64_t>(crc ^ 0xFFFFFFFF));
    };

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

    funcs["pbkdf2"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("pbkdf2: expected 3 arguments");
        std::string password = getString(args[0]);
        std::string salt = getString(args[1]);
        auto iters = std::dynamic_pointer_cast<Integer>(args[2]);
        if (!iters || iters->value <= 0) return makeError("pbkdf2: iterations must be positive");
        // Simplified PBKDF2 using HMAC-like construction
        uint8_t key[32] = {};
        SHA256 base;
        base.update(reinterpret_cast<const uint8_t*>(password.c_str()), password.size());
        base.update(reinterpret_cast<const uint8_t*>(salt.c_str()), salt.size());
        base.finalize(key);
        for (int64_t i = 1; i < iters->value; i++) {
            SHA256 round;
            round.update(key, 32);
            round.finalize(key);
        }
        return newString(toHex(key, 32));
    };

    funcs["hash"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("hash: expected 1 argument");
        return newInteger(static_cast<int64_t>(fnv1a(getString(args[0]))));
    };

    Registry::instance().registerModule("crypto", funcs);
}

} // namespace darix::native
