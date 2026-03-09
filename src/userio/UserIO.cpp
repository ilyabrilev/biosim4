#include "UserIO.h"
#include "SFMLUserIO.h"
#include "imageWriter.h"
#include "WebUserIO.h"

namespace BS {

    UserIO::UserIO(ViewMode viewMode, bool videoSaveInit)
    {
        switch (viewMode) {
        case ViewMode::SFML:
            this->sfmlView = new SFMLUserIO();
            break;
        case ViewMode::WEB:
            this->webView = new WebUserIO();
            break;
        case ViewMode::NONE:
            break;
        }
        if (videoSaveInit)
            this->imageWriter = new ImageWriter();
    }

    UserIO::~UserIO()
    {
        delete this->sfmlView;
        delete this->imageWriter;
        delete this->webView;
    }

    /**
     * Check if the simulation should be stopped (e.g. because of closed window)
     */
    bool UserIO::isStopped()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->isStopped();
        if (this->webView != nullptr)
            return this->webView->isStopped();
        return false;
    }

    /**
     * Should be called at the end of each generation
     */
    void UserIO::startNewGeneration(unsigned generation, unsigned stepsPerGeneration)
    {
        if (this->sfmlView != nullptr)
            this->sfmlView->startNewGeneration(generation, stepsPerGeneration);
        if (this->webView != nullptr)
            this->webView->startNewGeneration(generation, stepsPerGeneration);

        if (p.autoSave)
        {
            std::stringstream filename;
            filename << "Output/Saves/peeps-"
                        << std::setfill('0') << std::setw(6) << generation
                        << ".bin";
            Save::save(filename.str());
        }
    }

    /**
     * Should be called at the end of each step
     */
    void UserIO::handleStep(unsigned simStep, unsigned generation)
    {
        if (this->sfmlView != nullptr)
        {
            // handle pause/unpause
            while (this->isPaused() && !this->isStopped() ) {
                this->sfmlView->endOfStep(simStep);
            }
            this->sfmlView->endOfStep(simStep);
        }

        if (this->webView != nullptr)
        {
            while (this->isPaused() && !this->isStopped()) {
                this->webView->endOfStep(simStep);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            this->webView->endOfStep(simStep);
        }

        if (this->imageWriter != nullptr)
            this->imageWriter->endOfStep(simStep, generation);
    }

    /**
     * Should be called at the end of each generation
     */
    void UserIO::endOfGeneration(unsigned generation)
    {
        if (this->sfmlView != nullptr)
            this->sfmlView->endOfGeneration(generation);
        if (this->webView != nullptr)
            this->webView->endOfGeneration(generation);
        if (this->imageWriter != nullptr)
            this->imageWriter->endOfGeneration(generation);

        if (p.updateGraphLog && (generation == 1 || ((generation % p.updateGraphLogStride) == 0))) {
            std::system(p.graphLogUpdateCommand.c_str());
        }
    }

    void UserIO::log(std::string message)
    {
        if (this->sfmlView != nullptr)
            this->sfmlView->log(message);
        if (this->webView != nullptr)
            this->webView->log(message);

        std::cout << message << std::endl;
    }

    /**
     * Check if the simulation is paused
     */
    bool UserIO::isPaused()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->isPaused() && !this->sfmlView->loadFileSelected;
        if (this->webView != nullptr)
            return this->webView->isPaused() && !this->webView->loadFileSelected;
        return false;
    }

    /**
     * Check if a simulation should be switched into "load simulation" mode
     */
    bool UserIO::getLoadFileSelected()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->loadFileSelected;
        if (this->webView != nullptr)
            return this->webView->loadFileSelected;
        return false;
    }

    /**
     * Get the filename of the simulation to load
     */
    std::string UserIO::getLoadFilename()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->loadFilename;
        if (this->webView != nullptr)
            return this->webView->loadFilename;
        return "";
    }

    /**
     * Clean loading selection after loading a simulation
     */
    void UserIO::cleanLoadSelection()
    {
        if (this->sfmlView != nullptr) {
            this->sfmlView->loadFilename = "";
            this->sfmlView->loadFileSelected = false;
        }
        if (this->webView != nullptr) {
            this->webView->loadFilename = "";
            this->webView->loadFileSelected = false;
        }
    }

    /**
     * Update the view from the parameters
     */
    void UserIO::setFromParams()
    {
        if (this->sfmlView != nullptr)
            this->sfmlView->setFromParams();
        if (this->webView != nullptr)
            this->webView->setFromParams();
    }

    /**
     * Check if the simulation should restart at the end of this generation
     */
    bool UserIO::getRestartAtEnd()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->restartOnEnd;
        if (this->webView != nullptr)
            return this->webView->restartOnEnd;
        return false;
    }
}
