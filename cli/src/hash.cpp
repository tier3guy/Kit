#include "hash.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace kit {

namespace {

std::uint32_t rol(std::uint32_t value, int bits) {
    return (value << bits) | (value >> (32 - bits));
}

} // namespace

std::string sha1_hex(const std::string& data) {
    std::uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476,
                  h4 = 0xC3D2E1F0;

    std::string msg = data;
    const std::uint64_t bit_len = static_cast<std::uint64_t>(data.size()) * 8;

    msg += static_cast<char>(0x80);
    while (msg.size() % 64 != 56) {
        msg += static_cast<char>(0x00);
    }
    for (int i = 7; i >= 0; --i) {
        msg += static_cast<char>((bit_len >> (i * 8)) & 0xFF);
    }

    for (std::size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        std::array<std::uint32_t, 80> w{};
        for (int i = 0; i < 16; ++i) {
            const auto byte = [&](int off) {
                return static_cast<std::uint32_t>(static_cast<std::uint8_t>(msg[chunk + i * 4 + off]));
            };
            w[i] = (byte(0) << 24) | (byte(1) << 16) | (byte(2) << 8) | byte(3);
        }
        for (int i = 16; i < 80; ++i) {
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        std::uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        for (int i = 0; i < 80; ++i) {
            std::uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            const std::uint32_t temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(8) << h0 << std::setw(8) << h1
        << std::setw(8) << h2 << std::setw(8) << h3 << std::setw(8) << h4;
    return out.str();
}

} // namespace kit
