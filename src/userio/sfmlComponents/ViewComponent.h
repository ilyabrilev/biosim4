#ifndef SFMLVIEW_H_INCLUDED
#define SFMLVIEW_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

namespace BS
{
    /*
    * \brief Class for handling view of simulation
    */
    class ViewComponent
    {
    public:
        ViewComponent(sf::Vector2u windowSize);
        ~ViewComponent();
        sf::View* getView() { return this->view; }
        void updateInput(sf::Event e, sf::Vector2i mousePosition);
    private:
        sf::View* view;

        int windowHeight;
        int windowWidth;
        bool viewIsMoving = false;
        sf::Vector2f oldViewPos;
        double accumZoom = 1;
    };
}

#endif // SFMLVIEW_H_INCLUDED