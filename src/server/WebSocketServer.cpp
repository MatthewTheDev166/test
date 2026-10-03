#include "WebSocketServer.hpp"
#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

namespace {
    // --- Minimal self-contained SHA-1 implementation for RFC 6455 handshake ---
    struct SHA1Context {
        uint32_t state[5];
        uint32_t count[2];
        uint8_t buffer[64];
    };

    #define ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

    void sha1Transform(uint32_t state[5], const uint8_t buffer[64]) {
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
        uint32_t block[80];

        for (int i = 0; i < 16; i++) {
            block[i] = (static_cast<uint32_t>(buffer[4 * i + 0]) << 24)
                     | (static_cast<uint32_t>(buffer[4 * i + 1]) << 16)
                     | (static_cast<uint32_t>(buffer[4 * i + 2]) << 8)
                     | (static_cast<uint32_t>(buffer[4 * i + 3]));
        }
        for (int i = 16; i < 80; i++) {
            block[i] = ROL(block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16], 1);
        }

        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
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
            uint32_t temp = ROL(a, 5) + f + e + k + block[i];
            e = d;
            d = c;
            c = ROL(b, 30);
            b = a;
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }

    void sha1Init(SHA1Context* context) {
        context->state[0] = 0x67452301;
        context->state[1] = 0xEFCDAB89;
        context->state[2] = 0x98BADCFE;
        context->state[3] = 0x10325476;
        context->state[4] = 0xC3D2E1F0;
        context->count[0] = context->count[1] = 0;
    }

    void sha1Update(SHA1Context* context, const uint8_t* data, size_t len) {
        size_t i = 0;
        size_t j = (context->count[0] >> 3) & 63;
        if ((context->count[0] += static_cast<uint32_t>(len << 3)) < (static_cast<uint32_t>(len << 3))) {
            context->count[1]++;
        }
        context->count[1] += static_cast<uint32_t>(len >> 29);
        if ((j + len) > 63) {
            std::memcpy(&context->buffer[j], data, (i = 64 - j));
            sha1Transform(context->state, context->buffer);
            for (; i + 63 < len; i += 64) {
                sha1Transform(context->state, &data[i]);
            }
            j = 0;
        }
        std::memcpy(&context->buffer[j], &data[i], len - i);
    }

    void sha1Final(uint8_t digest[20], SHA1Context* context) {
        uint8_t finalcount[8];
        for (int i = 0; i < 8; i++) {
            finalcount[i] = static_cast<uint8_t>((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
        }
        uint8_t c = 0200;
        sha1Update(context, &c, 1);
        while ((context->count[0] & 504) != 448) {
            c = 0;
            sha1Update(context, &c, 1);
        }
        sha1Update(context, finalcount, 8);
        for (int i = 0; i < 20; i++) {
            digest[i] = static_cast<uint8_t>((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
        }
    }

    // --- Base64 Encoding ---
    static const char base64Chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string base64Encode(const uint8_t* bytes, size_t len) {
        std::string out;
        int val = 0, valb = -6;
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + bytes[i];
            valb += 8;
            while (valb >= 0) {
                out.push_back(base64Chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            out.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (out.size() % 4) {
            out.push_back('=');
        }
        return out;
    }

    std::string calculateAcceptKey(const std::string& clientKey) {
        std::string magic = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        SHA1Context ctx;
        sha1Init(&ctx);
        sha1Update(&ctx, reinterpret_cast<const uint8_t*>(magic.data()), magic.size());
        uint8_t digest[20];
        sha1Final(digest, &ctx);
        return base64Encode(digest, 20);
    }
}

WebSocketServer& WebSocketServer::get() {
    static WebSocketServer instance;
    return instance;
}

WebSocketServer::WebSocketServer() = default;

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(uint16_t port) {
    if (m_running.load()) {
        if (m_port == port) return true;
        stop();
    }

    m_port = port;

#ifdef GEODE_IS_WINDOWS
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        geode::log::error("Failed to initialize Winsock");
        return false;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        geode::log::error("Failed to create socket: {}", WSAGetLastError());
        WSACleanup();
        return false;
    }

    // Set reuse address
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(s, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        geode::log::error("Failed to bind socket to port {}: {}", port, WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return false;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        geode::log::error("Failed to listen on socket: {}", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return false;
    }

    m_listenSocket = static_cast<uintptr_t>(s);
    m_running.store(true);

    m_serverThread = std::thread(&WebSocketServer::serverLoop, this);
    geode::log::info("WebSocket server started on port {}", port);
    return true;
#else
    return false;
#endif
}

void WebSocketServer::stop() {
    if (!m_running.load()) return;
    m_running.store(false);

#ifdef GEODE_IS_WINDOWS
    if (m_listenSocket != static_cast<uintptr_t>(INVALID_SOCKET)) {
        closesocket(static_cast<SOCKET>(m_listenSocket));
        m_listenSocket = static_cast<uintptr_t>(INVALID_SOCKET);
    }

    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto client : m_clients) {
            closesocket(static_cast<SOCKET>(client));
        }
        m_clients.clear();
    }

    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }

    WSACleanup();
#endif
    geode::log::info("WebSocket server stopped");
}

bool WebSocketServer::isRunning() const {
    return m_running.load();
}

size_t WebSocketServer::getClientCount() {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

void WebSocketServer::setOnMessage(std::function<void(const std::string&)> callback) {
    m_onMessage = std::move(callback);
}

void WebSocketServer::serverLoop() {
#ifdef GEODE_IS_WINDOWS
    SOCKET listenSock = static_cast<SOCKET>(m_listenSocket);

    while (m_running.load()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSock, &readSet);

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000; // 200ms

        int selectRes = select(0, &readSet, nullptr, nullptr, &timeout);
        if (selectRes > 0 && FD_ISSET(listenSock, &readSet)) {
            sockaddr_in clientAddr{};
            int addrLen = sizeof(clientAddr);
            SOCKET clientSock = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
            if (clientSock != INVALID_SOCKET) {
                std::thread(&WebSocketServer::handleClient, this, static_cast<uintptr_t>(clientSock)).detach();
            }
        }
    }
#endif
}

bool WebSocketServer::doHandshake(uintptr_t clientSocket, const std::string& request) {
    std::istringstream stream(request);
    std::string line;
    std::string clientKey;

    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string keyHeader = "Sec-WebSocket-Key:";
        if (line.compare(0, keyHeader.size(), keyHeader) == 0) {
            clientKey = line.substr(keyHeader.size());
            // trim whitespace
            clientKey.erase(0, clientKey.find_first_not_of(" \t"));
            clientKey.erase(clientKey.find_last_not_of(" \t") + 1);
        }
    }

    if (clientKey.empty()) {
        return false;
    }

    std::string acceptKey = calculateAcceptKey(clientKey);
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << acceptKey << "\r\n\r\n";

    std::string respStr = response.str();
    int sent = send(static_cast<SOCKET>(clientSocket), respStr.data(), static_cast<int>(respStr.size()), 0);
    return (sent > 0);
}

void WebSocketServer::handleClient(uintptr_t clientSocket) {
#ifdef GEODE_IS_WINDOWS
    SOCKET sock = static_cast<SOCKET>(clientSocket);

    // 1. Read HTTP Handshake
    char buffer[4096];
    int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        closesocket(sock);
        return;
    }
    buffer[bytesRead] = '\0';

    if (!doHandshake(clientSocket, std::string(buffer, bytesRead))) {
        closesocket(sock);
        return;
    }

    // Handshake successful, add to connected clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.push_back(clientSocket);
    }
    geode::log::info("Mobile client connected via WebSocket! (Total: {})", getClientCount());

    // 2. Frame processing loop
    std::vector<uint8_t> frameBuffer;
    while (m_running.load()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int sel = select(0, &readSet, nullptr, nullptr, &timeout);
        if (sel <= 0) {
            if (sel < 0) break; // socket error
            continue; // timeout, keep waiting
        }

        uint8_t chunk[2048];
        int n = recv(sock, reinterpret_cast<char*>(chunk), sizeof(chunk), 0);
        if (n <= 0) {
            break; // disconnected
        }

        frameBuffer.insert(frameBuffer.end(), chunk, chunk + n);

        while (frameBuffer.size() >= 2) {
            uint8_t byte0 = frameBuffer[0];
            uint8_t byte1 = frameBuffer[1];

            uint8_t opcode = byte0 & 0x0F;
            bool isMasked = (byte1 & 0x80) != 0;
            uint64_t payloadLen = byte1 & 0x7F;

            size_t headerSize = 2;
            if (payloadLen == 126) {
                if (frameBuffer.size() < 4) break;
                payloadLen = (static_cast<uint64_t>(frameBuffer[2]) << 8) | frameBuffer[3];
                headerSize = 4;
            } else if (payloadLen == 127) {
                if (frameBuffer.size() < 10) break;
                payloadLen = 0;
                for (int i = 0; i < 8; i++) {
                    payloadLen = (payloadLen << 8) | frameBuffer[2 + i];
                }
                headerSize = 10;
            }

            if (isMasked) {
                headerSize += 4;
            }

            if (frameBuffer.size() < headerSize + payloadLen) {
                // Wait for more data
                break;
            }

            // Extract payload
            std::vector<uint8_t> payloadData(payloadLen);
            const uint8_t* maskKey = isMasked ? &frameBuffer[headerSize - 4] : nullptr;

            for (uint64_t i = 0; i < payloadLen; i++) {
                uint8_t b = frameBuffer[headerSize + i];
                if (isMasked) {
                    b ^= maskKey[i % 4];
                }
                payloadData[i] = b;
            }

            // Remove processed frame from buffer
            frameBuffer.erase(frameBuffer.begin(), frameBuffer.begin() + headerSize + payloadLen);

            if (opcode == 0x08) { // Connection Close
                goto client_disconnect;
            } else if (opcode == 0x09) { // Ping -> send Pong
                uint8_t pongHeader[2] = {0x8A, 0x00};
                send(sock, reinterpret_cast<const char*>(pongHeader), 2, 0);
            } else if (opcode == 0x01) { // Text frame
                std::string textMsg(payloadData.begin(), payloadData.end());
                if (m_onMessage) {
                    m_onMessage(textMsg);
                }
            }
        }
    }

client_disconnect:
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), clientSocket), m_clients.end());
    }
    closesocket(sock);
    geode::log::info("Mobile client disconnected. (Remaining: {})", getClientCount());
#endif
}

void WebSocketServer::sendFrame(uintptr_t clientSocket, const std::string& text) {
#ifdef GEODE_IS_WINDOWS
    SOCKET sock = static_cast<SOCKET>(clientSocket);
    std::vector<uint8_t> frame;

    frame.push_back(0x81); // FIN + Text opcode

    size_t len = text.size();
    if (len <= 125) {
        frame.push_back(static_cast<uint8_t>(len));
    } else if (len <= 65535) {
        frame.push_back(126);
        frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
        }
    }

    frame.insert(frame.end(), text.begin(), text.end());
    send(sock, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
#endif
}

void WebSocketServer::broadcast(const std::string& text) {
#ifdef GEODE_IS_WINDOWS
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (auto client : m_clients) {
        sendFrame(client, text);
    }
#endif
}
