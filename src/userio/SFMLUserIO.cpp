#include "SFMLUserIO.h"

namespace BS
{
    SFMLUserIO::SFMLUserIO()
    {
        this->window = new sf::RenderWindow(sf::VideoMode(this->windowWidth, this->windowHeight), "biosim4", sf::Style::Close | sf::Style::Titlebar);
        this->window->setFramerateLimit(144);
        this->window->setVerticalSyncEnabled(false);
        this->window->setPosition(sf::Vector2i(200, (sf::VideoMode::getDesktopMode().height - this->windowHeight) / 2));

        // setup gui
        this->gui.setWindow(*this->window);
        tgui::Theme::setDefault("./Resources/Black.txt");

        // setup left panel
        this->rightPanelComponent = new RightPanelComponent(
            this->window->getSize(),
            [this](bool paused)
            {
                this->pauseResume(paused);
            },
            [this](std::string name, std::string val)
            {
                this->settingsChanged(name, val);
            });

        this->rightPanelComponent->initSpeedControls(SPEED_SLOW_MAX, SPEED_FAST_MAX, 0, [this](float value) { this->speedChanged(value); });
        this->rightPanelComponent->initSaveLoadButtons(
            [this](void)
            {
                if (this->isFileDialogShowing)
                {
                    return;
                }
                
                if (this->saveFileDialog == nullptr) 
                {
                    this->saveFileDialog = tgui::FileDialog::create("Save file", "Save");
                    this->saveFileDialog->setMultiSelect(false);
                    this->saveFileDialog->setFileMustExist(false);
                    this->saveFileDialog->setPath("Output/Saves");
                    this->saveFileDialog->setFilename("simulation.bin");
                    //this->saveFileDialog->setFileTypeFilters({ {"All files", {}} }, 1);
                    this->saveFileDialog->setPosition("5%", "5%");

                    this->saveFileDialog->onFileSelect([this](const tgui::String& filePath){
                        Save::save(filePath.toStdString());
                        this->fileDialogToggled(false);
                    });

                    this->saveFileDialog->onCancel([this]{
                        this->fileDialogToggled(false);
                    });
                }
                this->saveFileDialog->setPath("Output/Saves"); // update files list
                this->fileDialogToggled(true);
                this->gui.add(this->saveFileDialog);
            },
            [this]()
            {
                if (this->isFileDialogShowing)
                {
                    return;
                }

                if (this->loadFileDialog == nullptr) {
                    this->loadFileDialog = tgui::FileDialog::create("Open file", "Open");
                    this->loadFileDialog->setMultiSelect(false);
                    this->loadFileDialog->setFileMustExist(true);
                    this->loadFileDialog->setPath("Output/Saves");
                    this->loadFileDialog->setFilename("simulation.bin");
                    //this->saveFileDialog->setFileTypeFilters({ {"All files", {}} }, 1);
                    this->loadFileDialog->setPosition("5%", "5%");

                    this->loadFileDialog->onFileSelect([this](const tgui::String& filePath){
                        this->loadFileSelected = true;
                        this->loadFilename = filePath.toStdString();
                        this->fileDialogToggled(false);
                    });
                    this->loadFileDialog->onCancel([this]{
                        std::cerr << "No file selected.\n";
                        this->fileDialogToggled(false);
                    });    
                }

                this->loadFileDialog->setPath("Output/Saves"); // update files list
                this->fileDialogToggled(true);
                this->gui.add(this->loadFileDialog);
            }
        );

        this->gui.add(this->rightPanelComponent->getPanel());

        // setup view
        this->viewComponent = new ViewComponent(this->window->getSize());
        this->view = this->viewComponent->getView();
    }

    void SFMLUserIO::setFromParams()
    {
        this->rightPanelComponent->setFromParams();
    }

    void SFMLUserIO::fileDialogToggled(bool shown)
    {
        if (shown)
        {
            this->viewComponent->lock();
        }
        else 
        {
            this->viewComponent->unlock();
        }
        this->rightPanelComponent->pauseExternal(shown);
        this->isFileDialogShowing = shown;
    }

    SFMLUserIO::~SFMLUserIO()
    {
        delete this->window;
    }

    bool SFMLUserIO::isStopped()
    {
        return !this->window->isOpen();
    }

    void SFMLUserIO::updatePollEvents()
    {
        sf::Event e;
        while (this->window->pollEvent(e))
        {
            if (e.Event::type == sf::Event::Closed)
            {
                this->window->close();
            }

            if (e.Event::type == e.Event::KeyPressed && e.Event::key.code == sf::Keyboard::Key::Escape)
            {
                this->window->close();
            }

            this->viewComponent->updateInput(e, sf::Mouse::getPosition(*this->window));

            this->gui.handleEvent(e);
        }
    }

    void SFMLUserIO::startNewGeneration(unsigned generation, unsigned stepsPerGeneration)
    {
        this->rightPanelComponent->startNewGeneration(generation, stepsPerGeneration);
        this->updatePollEvents();
    }

    void SFMLUserIO::endOfStep(unsigned simStep)
    {
        // handle increase speed by skipping frames
        if (this->increaseSpeedCounter < this->speedThreshold)
        {
            this->increaseSpeedCounter++;
            return;
        }
        this->increaseSpeedCounter = 0;

        do 
        {
            this->updatePollEvents();

            this->rightPanelComponent->endOfStep(simStep);
            this->window->setView(*this->view);

            this->window->clear();

            //display population
            int liveDisplayScale = p.displayScale / 1.5;
            for (uint16_t index = 1; index <= p.population; ++index)
            {
                Indiv &indiv = peeps[index];
                if (indiv.alive)
                {
                    indiv.shape.setPosition(
                        static_cast<float>(indiv.loc.x * liveDisplayScale),
                        static_cast<float>(((p.sizeY - indiv.loc.y) - 1) * liveDisplayScale));
                    this->window->draw(indiv.shape);
                }
            }

            this->gui.draw();

            this->window->display();

            //handle slow time by drawing speedThreshold unchanged frames
            this->slowSpeedCounter--;

        } while (this->slowSpeedCounter >= this->speedThreshold);
        this->slowSpeedCounter = 0;
    }

    void SFMLUserIO::endOfGeneration(unsigned generation)
    {
    }

    void SFMLUserIO::log(std::string message)
    {
        this->rightPanelComponent->log(message);
    }

    void SFMLUserIO::pauseResume(bool paused)
    {
        this->paused = paused;
    }

    bool SFMLUserIO::isPaused()
    {
        return this->paused;
    }

    void SFMLUserIO::settingsChanged(std::string name, std::string val)
    {
        paramManager.changeFromUi(name, val);
    }

    void SFMLUserIO::speedChanged(float value)
    {
        this->speedThreshold = value;
    }
}