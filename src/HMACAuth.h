#pragma once
#include <string>
#include <Geode/utils/web.hpp>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <matjson.hpp>
#include <cstring>
#include <cstdint>
#include <vector>
#include <array>

namespace HMACAuth {

  
    namespace detail {

        static constexpr uint32_t K[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

        inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
        inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
        inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
        inline uint32_t ep0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
        inline uint32_t ep1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
        inline uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
        inline uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

        inline std::array<uint8_t, 32> sha256(const uint8_t* data, size_t len) {
            uint32_t h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372, h3 = 0xa54ff53a;
            uint32_t h4 = 0x510e527f, h5 = 0x9b05688c, h6 = 0x1f83d9ab, h7 = 0x5be0cd19;

            uint64_t bitLen = (uint64_t)len * 8;
            size_t padLen = len + 1;
            while (padLen % 64 != 56) padLen++;
            padLen += 8;

            std::vector<uint8_t> msg(padLen, 0);
            std::memcpy(msg.data(), data, len);
            msg[len] = 0x80;
            for (int i = 0; i < 8; i++) {
                msg[padLen - 1 - i] = (uint8_t)(bitLen >> (i * 8));
            }

            for (size_t offset = 0; offset < padLen; offset += 64) {
                uint32_t w[64];
                for (int i = 0; i < 16; i++) {
                    w[i] = ((uint32_t)msg[offset + i * 4] << 24) |
                        ((uint32_t)msg[offset + i * 4 + 1] << 16) |
                        ((uint32_t)msg[offset + i * 4 + 2] << 8) |
                        ((uint32_t)msg[offset + i * 4 + 3]);
                }
                for (int i = 16; i < 64; i++) {
                    w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
                }

                uint32_t a = h0, b = h1, c = h2, d = h3;
                uint32_t e = h4, f = h5, g = h6, h = h7;

                for (int i = 0; i < 64; i++) {
                    uint32_t t1 = h + ep1(e) + ch(e, f, g) + K[i] + w[i];
                    uint32_t t2 = ep0(a) + maj(a, b, c);
                    h = g; g = f; f = e; e = d + t1;
                    d = c; c = b; b = a; a = t1 + t2;
                }

                h0 += a; h1 += b; h2 += c; h3 += d;
                h4 += e; h5 += f; h6 += g; h7 += h;
            }

            std::array<uint8_t, 32> hash;
            uint32_t hs[] = { h0, h1, h2, h3, h4, h5, h6, h7 };
            for (int i = 0; i < 8; i++) {
                hash[i * 4] = (uint8_t)(hs[i] >> 24);
                hash[i * 4 + 1] = (uint8_t)(hs[i] >> 16);
                hash[i * 4 + 2] = (uint8_t)(hs[i] >> 8);
                hash[i * 4 + 3] = (uint8_t)(hs[i]);
            }
            return hash;
        }

        inline std::array<uint8_t, 32> hmacSha256(
            const uint8_t* key, size_t keyLen,
            const uint8_t* msg, size_t msgLen
        ) {
            const size_t blockSize = 64;
            std::array<uint8_t, 64> keyBlock = {};

            if (keyLen > blockSize) {
                auto hashed = sha256(key, keyLen);
                std::memcpy(keyBlock.data(), hashed.data(), 32);
            }
            else {
                std::memcpy(keyBlock.data(), key, keyLen);
            }

            std::array<uint8_t, 64> ipad, opad;
            for (size_t i = 0; i < blockSize; i++) {
                ipad[i] = keyBlock[i] ^ 0x36;
                opad[i] = keyBlock[i] ^ 0x5c;
            }

            std::vector<uint8_t> inner(blockSize + msgLen);
            std::memcpy(inner.data(), ipad.data(), blockSize);
            std::memcpy(inner.data() + blockSize, msg, msgLen);
            auto innerHash = sha256(inner.data(), inner.size());

            std::vector<uint8_t> outer(blockSize + 32);
            std::memcpy(outer.data(), opad.data(), blockSize);
            std::memcpy(outer.data() + blockSize, innerHash.data(), 32);
            return sha256(outer.data(), outer.size());
        }

    }  

 
    inline std::string s_sessionToken = "";
    inline void setSessionToken(const std::string& token) { s_sessionToken = token; }
    inline std::string getSessionToken() { return s_sessionToken; }
    inline void clearSessionToken() { s_sessionToken = ""; }

 
    inline std::string getSecret() {
        return "bsjsbdisbiaamqyquisbksbmlaldadwdscveamdmdcnmzlpqwalljcnmhsapaokdzmmnddjuenpxmse";
    }

  
    inline std::string toHex(const uint8_t* data, size_t len) {
        std::ostringstream oss;
        for (size_t i = 0; i < len; i++) {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        }
        return oss.str();
    }

    inline std::string computeHMAC(const std::string& message) {
        std::string secret = getSecret();
        auto hash = detail::hmacSha256(
            reinterpret_cast<const uint8_t*>(secret.data()), secret.size(),
            reinterpret_cast<const uint8_t*>(message.data()), message.size()
        );
        return toHex(hash.data(), hash.size());
    }

    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();
        return std::to_string(epoch);
    }

 
    inline void signRequest(
        web::WebRequest& req,
        int accountID,
        const matjson::Value& body = matjson::Value::object()
    ) {
        std::string accountStr = std::to_string(accountID);
        std::string timestamp = getTimestamp();
        std::string bodyStr = body.dump(matjson::NO_INDENTATION);

        std::string message = accountStr + ":" + timestamp + ":" + bodyStr;
        std::string signature = computeHMAC(message);

        req.header("X-Account-ID", accountStr);
        req.header("X-Timestamp", timestamp);
        req.header("X-Signature", signature);

        if (!s_sessionToken.empty()) {
            req.header("X-Session-Token", s_sessionToken);
        }
    }

    inline void signGetRequest(web::WebRequest& req, int accountID) {
        signRequest(req, accountID, matjson::Value::object());
    }
}