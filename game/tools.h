#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using str = std::string;

sf::Color background(0, 0, 0), ChangeColor(1, 1, 1, 0), textColor(255, 255, 255);
int scw = sf::VideoMode::getDesktopMode().width;
int sch = sf::VideoMode::getDesktopMode().height;

int seconds, counter;

sf::RenderWindow window(sf::VideoMode(scw, sch), "unnamed", sf::Style::Fullscreen);
sf::Event event;
sf::Font font;
sf::Text text;
sf::Music birds, metronom, machine, whoosh;
sf::Mouse mouse;

sf::CircleShape BlackCircle;

enum stages {
    intro,
    HelloStranger,
    NobodyHere,
    You,
    YouHere,
    YouHereWait,
    Here,
    HereWait,
    BlackHole,
    end1,
    end2
};

stages curStage = stages::intro;
