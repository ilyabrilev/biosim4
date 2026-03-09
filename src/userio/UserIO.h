#ifndef USERIO_H_INCLUDED
#define USERIO_H_INCLUDED

// Operate with view of simulation through console, images, window, or browser

#include "../peeps.h"
#include "../utils/Save.h"

namespace BS
{
    extern const Params &p;

    // Forward declarations to avoid pulling SFML/uWS headers into every TU
    class SFMLUserIO;
    class ImageWriter;
    class WebUserIO;

    enum class ViewMode { SFML, WEB, NONE };

    /**
     * UserIO class handles all simulation events that must be displayed
     * and passed to imageWriter, SFMLUserIO, or WebUserIO classes
     */
    class UserIO
    {
    public:
        UserIO(ViewMode viewMode, bool videoSaveInit);
        ~UserIO();

        bool isStopped();
        void handleStep(unsigned simStep, unsigned generation);
        void endOfGeneration(unsigned generation);
        void startNewGeneration(unsigned generation, unsigned stepsPerGeneration);

        void log(std::string message);

        bool isPaused();

        bool getLoadFileSelected();
        std::string getLoadFilename();
        void cleanLoadSelection();

        void setFromParams();

        bool getRestartAtEnd();
    private:
        ImageWriter* imageWriter = nullptr;
        SFMLUserIO* sfmlView = nullptr;
        WebUserIO* webView = nullptr;
    };
}

#endif // USERIO_H_INCLUDED