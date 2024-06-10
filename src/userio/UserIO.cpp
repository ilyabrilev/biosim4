#include "UserIO.h"

namespace BS {

    UserIO::UserIO(bool windowInit, bool videoSaveInit)
    {
        if (windowInit) 
            this->sfmlView = new SFMLUserIO();
        if (videoSaveInit) 
            this->imageWriter = new ImageWriter();        
    }

    UserIO::~UserIO()
    {
        if (this->sfmlView != nullptr)
            delete this->sfmlView;
        if (this->imageWriter != nullptr)
            delete this->imageWriter;
    }

    bool UserIO::isStopped()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->isStopped();
        return false;
    }

    void UserIO::startNewGeneration(unsigned generation, unsigned stepsPerGeneration)
    {
        if (this->sfmlView != nullptr) {
            this->sfmlView->startNewGeneration(generation, stepsPerGeneration);
        }
    }

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

        if (this->imageWriter != nullptr)
            this->imageWriter->endOfStep(simStep, generation);
    }

    void UserIO::endOfGeneration(unsigned generation)
    {
        if (this->sfmlView != nullptr)
            this->sfmlView->endOfGeneration(generation);

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
        
        std::cout << message << std::endl;
    }

    bool UserIO::isPaused()
    {
        if (this->sfmlView != nullptr)
            return this->sfmlView->isPaused();

        return false;
    }
}