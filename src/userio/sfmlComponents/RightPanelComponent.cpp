#include "RightPanelComponent.h"
#include "../../simulator.h"

namespace BS
{
    RightPanelComponent::RightPanelComponent(
        sf::Vector2u windowSize, 
        std::function<void(std::string name, std::string val)> changeSettingsCallback_,
        std::function<void()> infoCallback
    )
    {
        this->changeSettingsCallback = changeSettingsCallback_;

        // create panel
        this->panel = tgui::Panel::create();
        this->panel->setSize("20%", windowSize.y);
        this->panel->setAutoLayout(tgui::AutoLayout::Right);

        this->initSettingsComponents(infoCallback);
    }

    void RightPanelComponent::initSettingsComponents(std::function<void()> infoCallback)
    {
        // setup challenge box
        this->challengeBoxComponent = new ChallengeBoxComponent([this](std::string name, std::string val)
                                                                                 { this->changeSettingsCallback(name, val); });
        tgui::ComboBox::Ptr challengeBox = this->challengeBoxComponent->getComboBox();
        challengeBox->setPosition("5%", "15%");
        this->panel->add(challengeBox, "ChallengeBox");
        this->createLabel(challengeBox, "Challenge");

        tgui::Button::Ptr challengeInfoBtn = tgui::Button::create("i");
        challengeInfoBtn->setPosition({bindRight(challengeBox) + 5.f, bindTop(challengeBox)});
        challengeInfoBtn->onPress([infoCallback]()
                         { infoCallback(); });
        challengeInfoBtn->setHeight(challengeBox->getSize().y);
        this->panel->add(challengeInfoBtn, "IndivInfoBtn");

        // setup mutation rate
        this->mutationRateEditBox = tgui::EditBox::create();
        this->mutationRateEditBox->setPosition({bindLeft(challengeBox), bindBottom(challengeBox) + this->controlOffset});
        this->mutationRateEditBox->setText(tgui::String(p.pointMutationRate));
        this->panel->add(this->mutationRateEditBox);
        this->createLabel(this->mutationRateEditBox, "Mutation Rate");

        tgui::Button::Ptr mutationButton = tgui::Button::create("Ok");
        mutationButton->setPosition({bindRight(this->mutationRateEditBox) + 2.f, bindTop(this->mutationRateEditBox)});
        mutationButton->onPress([this]()
                                { this->changeSettingsCallback("pointmutationrate", this->mutationRateEditBox->getText().toStdString()); });
        mutationButton->setHeight(this->mutationRateEditBox->getSize().y);
        this->panel->add(mutationButton, "MutationButton");
        
        // setup barrier box
        this->barrierBoxComponent = new BarrierBoxComponent([this](std::string name, std::string val)
                                                                                 { this->changeSettingsCallback(name, val); });
        tgui::ComboBox::Ptr barrierBox = this->barrierBoxComponent->getComboBox();
        barrierBox->setPosition({bindLeft(this->mutationRateEditBox), bindBottom(this->mutationRateEditBox) + this->controlOffset});
        this->panel->add(barrierBox, "BarrierBox");
        this->createLabel(barrierBox, "Barrier");
    }

    void RightPanelComponent::setFromParams()
    {
        this->challengeBoxComponent->setFromParams();
        this->mutationRateEditBox->setText(tgui::String(p.pointMutationRate));
    }

    void RightPanelComponent::createLabel(tgui::Widget::Ptr widget, const tgui::String &text)
    {
        tgui::Label::Ptr label = tgui::Label::create(text);
        label->setPosition({bindLeft(widget), bindTop(widget) - this->labelOffset});
        this->panel->add(label);
    }

    RightPanelComponent::~RightPanelComponent()
    {
    }

    void RightPanelComponent::addToPanel(const tgui::Widget::Ptr &widgetPtr, const tgui::String &widgetName)
    {
        this->panel->add(widgetPtr, widgetName);
    }
}