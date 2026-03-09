#ifndef WEBUSERIO_H_INCLUDED
#define WEBUSERIO_H_INCLUDED

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <set>

#include <uWebSockets/src/App.h>

#include "../ai/indiv.h"
#include "../params.h"
#include "../peeps.h"
#include "../grid.h"
#include "../survivalCriteria/SurvivalCriteriaManager.h"

namespace BS
{
    extern Peeps peeps;
    extern Grid grid;
    extern const Params &p;
    extern ParamManager paramManager;
    extern SurvivalCriteriaManager survivalCriteriaManager;

    /**
     * WebSocket-based output for browser visualization.
     * Streams simulation state as binary data to connected clients.
     */
    class WebUserIO
    {
    public:
        WebUserIO(int port = 9002);
        ~WebUserIO();

        bool isStopped();
        bool isPaused();
        void pauseResume(bool paused);

        void startNewGeneration(unsigned generation, unsigned stepsPerGeneration);
        void endOfStep(unsigned simStep);
        void endOfGeneration(unsigned generation);

        void log(std::string message);

        bool loadFileSelected = false;
        std::string loadFilename;

        void setFromParams();
        bool restartOnEnd = false;

    private:
        struct PerSocketData {};

        std::thread serverThread;
        uWS::App *app = nullptr;
        uWS::Loop *loop = nullptr;
        struct us_listen_socket_t *listenSocket = nullptr;

        std::mutex clientsMutex;
        std::set<uWS::WebSocket<false, true, PerSocketData>*> clients;

        std::atomic<bool> stopped{false};
        std::atomic<bool> paused{false};

        int port;
        unsigned currentGeneration = 0;
        int speedThreshold = 0;
        int increaseSpeedCounter = 0;

        void startServer();
        void broadcastState(unsigned simStep, unsigned generation);
        void handleCommand(std::string_view message);

        // Builds binary frame with positions, colors, barriers, shapes
        std::vector<char> buildFrame(unsigned simStep, unsigned generation);

        // Builds JSON metadata sent to clients on connect
        std::string buildMetadata();
    };
}

#endif // WEBUSERIO_H_INCLUDED
