#include "WebUserIO.h"
#include <iostream>
#include <cstring>
#include <sstream>

namespace BS
{
    WebUserIO::WebUserIO(int port_) : port(port_)
    {
        startServer();
    }

    WebUserIO::~WebUserIO()
    {
        stopped = true;
        if (loop) {
            loop->defer([this]() {
                if (listenSocket) {
                    us_listen_socket_close(0, listenSocket);
                }
                // Close all client connections
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (auto *ws : clients) {
                    ws->close();
                }
            });
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void WebUserIO::startServer()
    {
        serverThread = std::thread([this]() {
            auto app = uWS::App();

            app.ws<PerSocketData>("/*", {
                .open = [this](auto *ws) {
                    {
                        std::lock_guard<std::mutex> lock(clientsMutex);
                        clients.insert(ws);
                    }
                    std::cout << "WebSocket client connected. Total: " << clients.size() << std::endl;
                    // Send metadata (challenge list, current params) as TEXT
                    std::string meta = buildMetadata();
                    ws->send(meta, uWS::OpCode::TEXT);
                },
                .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    handleCommand(message);
                },
                .close = [this](auto *ws, int code, std::string_view message) {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    clients.erase(ws);
                    std::cout << "WebSocket client disconnected. Total: " << clients.size() << std::endl;
                }
            });

            app.listen(port, [this](auto *token) {
                listenSocket = token;
                if (token) {
                    std::cout << "WebUserIO listening on port " << port << std::endl;
                } else {
                    std::cerr << "WebUserIO failed to listen on port " << port << std::endl;
                }
            });

            this->loop = uWS::Loop::get();
            app.run();
        });
    }

    void WebUserIO::handleCommand(std::string_view message)
    {
        std::string cmd(message);
        if (cmd == "pause") {
            paused = true;
        } else if (cmd == "resume") {
            paused = false;
        } else if (cmd == "restart") {
            restartOnEnd = true;
        } else if (cmd == "stop") {
            stopped = true;
        } else if (cmd.rfind("speed:", 0) == 0) {
            speedThreshold = std::stoi(cmd.substr(6));
        } else if (cmd.rfind("setting:", 0) == 0) {
            // format: "setting:name=value"
            auto payload = cmd.substr(8);
            auto eq = payload.find('=');
            if (eq != std::string::npos) {
                paramManager.changeFromUi(payload.substr(0, eq), payload.substr(eq + 1));
            }
        }
    }

    /**
     * Build binary frame for one simulation step.
     *
     * Format (little-endian):
     *   Header (16 bytes):
     *     uint32 simStep
     *     uint32 generation
     *     uint16 sizeX
     *     uint16 sizeY
     *     uint32 populationCount (alive)
     *   Per alive individual (7 bytes each):
     *     uint16 x
     *     uint16 y
     *     uint8  r, g, b
     *   Barrier section:
     *     uint32 barrierCount
     *     Per barrier (4 bytes):
     *       uint16 x
     *       uint16 y
     *   Shape section:
     *     uint32 shapeCount
     *     Per shape:
     *       uint8 type (0=circle, 1=rect, 2=line)
     *       followed by type-specific floats and color bytes
     */
    std::vector<char> WebUserIO::buildFrame(unsigned simStep, unsigned generation)
    {
        std::vector<char> buf;
        buf.reserve(16 + p.population * 7 + 1024);

        auto writeU32 = [&buf](uint32_t v) {
            buf.insert(buf.end(), reinterpret_cast<char*>(&v), reinterpret_cast<char*>(&v) + 4);
        };
        auto writeU16 = [&buf](uint16_t v) {
            buf.insert(buf.end(), reinterpret_cast<char*>(&v), reinterpret_cast<char*>(&v) + 2);
        };
        auto writeU8 = [&buf](uint8_t v) {
            buf.push_back(static_cast<char>(v));
        };
        auto writeFloat = [&buf](float v) {
            buf.insert(buf.end(), reinterpret_cast<char*>(&v), reinterpret_cast<char*>(&v) + 4);
        };

        // Header
        writeU32(simStep);
        writeU32(generation);
        writeU16(p.sizeX);
        writeU16(p.sizeY);

        // Count alive individuals
        uint32_t aliveCount = 0;
        for (uint16_t i = 1; i <= p.population; ++i) {
            if (peeps[i].alive) aliveCount++;
        }
        writeU32(aliveCount);

        // Individual positions and colors
        for (uint16_t i = 1; i <= p.population; ++i) {
            const Indiv &indiv = peeps[i];
            if (!indiv.alive) continue;

            writeU16(static_cast<uint16_t>(indiv.loc.x));
            writeU16(static_cast<uint16_t>(indiv.loc.y));

            uint8_t rawColor = indiv.makeGeneticColor();
            uint8_t r = rawColor;
            uint8_t g = (rawColor & 0x1f) << 3;
            uint8_t b = (rawColor & 7) << 5;

            constexpr uint8_t minColorVal = 100;
            constexpr uint8_t minLumaVal = 50;
            if ((r+r+r+b+g+g+g+g) / 8 < minLumaVal) {
                if (r < minColorVal) r = 255 - r;
                if (g < minColorVal) g = 255 - g;
                if (b < minColorVal) b = 255 - b;
            }

            writeU8(r);
            writeU8(g);
            writeU8(b);
        }

        // Barriers
        auto const &barrierLocs = grid.getBarrierLocations();
        writeU32(static_cast<uint32_t>(barrierLocs.size()));
        for (const Coord &loc : barrierLocs) {
            writeU16(static_cast<uint16_t>(loc.x));
            writeU16(static_cast<uint16_t>(loc.y));
        }

        // Challenge shapes
        const auto &shapes = survivalCriteriaManager.getShapes();
        writeU32(static_cast<uint32_t>(shapes.size()));
        for (const auto &shape : shapes) {
            std::visit([&](const auto &s) {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, ShapeCircle>) {
                    writeU8(0);
                    writeFloat(s.radius);
                    writeFloat(s.x);
                    writeFloat(s.y);
                    writeU8(s.outlineColor.r);
                    writeU8(s.outlineColor.g);
                    writeU8(s.outlineColor.b);
                    writeU8(s.outlineColor.a);
                } else if constexpr (std::is_same_v<T, ShapeRect>) {
                    writeU8(1);
                    writeFloat(s.x);
                    writeFloat(s.y);
                    writeFloat(s.width);
                    writeFloat(s.height);
                    writeU8(s.fillColor.r);
                    writeU8(s.fillColor.g);
                    writeU8(s.fillColor.b);
                    writeU8(s.fillColor.a);
                } else if constexpr (std::is_same_v<T, ShapeLine>) {
                    writeU8(2);
                    writeFloat(s.x1);
                    writeFloat(s.y1);
                    writeFloat(s.x2);
                    writeFloat(s.y2);
                    writeU8(s.color.r);
                    writeU8(s.color.g);
                    writeU8(s.color.b);
                    writeU8(s.color.a);
                }
            }, shape);
        }

        return buf;
    }

    void WebUserIO::broadcastState(unsigned simStep, unsigned generation)
    {
        if (!loop) return;

        auto frame = buildFrame(simStep, generation);

        loop->defer([this, frame = std::move(frame)]() {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::string_view data(frame.data(), frame.size());
            for (auto *ws : clients) {
                ws->send(data, uWS::OpCode::BINARY);
            }
        });
    }

    bool WebUserIO::isStopped()
    {
        return stopped;
    }

    bool WebUserIO::isPaused()
    {
        return paused && !loadFileSelected;
    }

    void WebUserIO::pauseResume(bool p)
    {
        paused = p;
    }

    void WebUserIO::startNewGeneration(unsigned generation, unsigned stepsPerGeneration)
    {
        currentGeneration = generation;
        increaseSpeedCounter = 0;
        // Initialize challenge shapes in grid coordinates (scale=1)
        survivalCriteriaManager.initShapes(1);
    }

    void WebUserIO::endOfStep(unsigned simStep)
    {
        // Skip frames based on speed setting
        if (increaseSpeedCounter < speedThreshold) {
            increaseSpeedCounter++;
            return;
        }
        increaseSpeedCounter = 0;

        broadcastState(simStep, currentGeneration);
    }

    void WebUserIO::endOfGeneration(unsigned generation)
    {
    }

    void WebUserIO::log(std::string message)
    {
        std::cout << "[Web] " << message << std::endl;
    }

    void WebUserIO::setFromParams()
    {
    }

    std::string WebUserIO::buildMetadata()
    {
        std::ostringstream ss;
        ss << "{\"type\":\"meta\",\"challenge\":" << p.challenge
           << ",\"pointMutationRate\":" << p.pointMutationRate
           << ",\"killEnable\":" << (p.killEnable ? "true" : "false")
           << ",\"population\":" << p.population
           << ",\"stepsPerGeneration\":" << p.stepsPerGeneration
           << ",\"barrierType\":" << p.barrierType
           << ",\"barriers\":["
           << "{\"value\":0,\"text\":\"No barrier\"}"
           << ",{\"value\":1,\"text\":\"Vertical const\"}"
           << ",{\"value\":2,\"text\":\"Vertical rand\"}"
           << ",{\"value\":3,\"text\":\"5 blocks staggered\"}"
           << ",{\"value\":4,\"text\":\"Horizontal const\"}"
           << ",{\"value\":5,\"text\":\"3 floating islands\"}"
           << ",{\"value\":6,\"text\":\"Spots\"}"
           << "],\"challenges\":[";

        const auto &vec = survivalCriteriaManager.survivalCriteriasVector;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) ss << ",";
            // Escape quotes in text/description
            auto escape = [](const std::string &s) {
                std::string out;
                for (char c : s) {
                    if (c == '"') out += "\\\"";
                    else if (c == '\\') out += "\\\\";
                    else if (c == '\n') out += "\\n";
                    else out += c;
                }
                return out;
            };
            ss << "{\"value\":" << vec[i]->value
               << ",\"text\":\"" << escape(vec[i]->text)
               << "\",\"description\":\"" << escape(vec[i]->description)
               << "\"}";
        }
        ss << "]}";
        return ss.str();
    }
}
