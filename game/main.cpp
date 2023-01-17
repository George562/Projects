#include "tools.h"
// -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

int main() {
    window.setVerticalSyncEnabled(true);
    setlocale(LC_ALL, "rus");

    window.setMouseCursorVisible(false);

    font.loadFromFile("font.ttf");
    text.setFont(font);
    text.setCharacterSize(40);
    text.setString("Привет, Странник");
    text.setPosition((scw - text.getGlobalBounds().width) / 2,
                     (sch - text.getGlobalBounds().height) / 2);

    birds.openFromFile("birds.ogg");
    metronom.openFromFile("metronom.ogg");
    machine.openFromFile("Machine_RhythmicSynth_Loop.wav");
    whoosh.openFromFile("Initial_Whoosh.wav");
    machine.setVolume(25);
    birds.play(); metronom.play(); machine.play();
    machine.setLoop(true); metronom.setLoop(true);

    window.setVerticalSyncEnabled(true);

    sf::Clock clock;
    sf::Time ElapsedTime, timer = sf::seconds(10);

    while (window.isOpen()) {

        ElapsedTime = clock.getElapsedTime();
        if (ElapsedTime < timer) {
            if (curStage == stages::intro) {
                background += ChangeColor;
                window.clear(background);
            } else if (curStage == stages::HelloStranger) {
                window.clear(background);
                textColor -= ChangeColor;
                text.setFillColor(textColor);
                window.draw(text);
            } else if (curStage == stages::NobodyHere) {
                window.clear(background);
                if (seconds != int(ElapsedTime.asSeconds())) {
                    seconds = int(ElapsedTime.asSeconds());
                    text.setString(text.getString().substring(1, text.getString().getSize() - 1));
                    text.setPosition((scw - text.getGlobalBounds().width) / 2,
                                    (sch - text.getGlobalBounds().height) / 2);
                    if (text.getString().getSize() == 2)
                        timer = ElapsedTime;
                }
                window.draw(text);
            } else if (curStage == stages::You) {
                window.clear(background);
                window.draw(text);
                metronom.setVolume(metronom.getVolume() - 0.09);
                machine.setVolume(metronom.getVolume() - 0.25);
            } else if (curStage == stages::YouHere) {
                window.clear(background);
                if (seconds != int(ElapsedTime.asSeconds())) {
                    text.setString(text.getString() + " один здесь?"[counter]);
                    text.setPosition((scw - text.getGlobalBounds().width) / 2,
                                    (sch - text.getGlobalBounds().height) / 2);
                    counter++;
                    seconds = int(ElapsedTime.asSeconds());
                    if (counter == 12)
                        timer = ElapsedTime;
                }
                metronom.setVolume(metronom.getVolume() - 0.05);
                window.draw(text);
            } else if (curStage == stages::YouHereWait) {
                window.clear(background);
                window.draw(text);
                metronom.setVolume(metronom.getVolume() - 0.05);
            } else if (curStage == stages::Here) {
                window.clear(background);
                if (seconds != int(ElapsedTime.asSeconds())) {
                    seconds = int(ElapsedTime.asSeconds());
                    text.setString(text.getString().substring(1, text.getString().getSize() - 1));
                    text.setPosition((scw - text.getGlobalBounds().width) / 2,
                                    (sch - text.getGlobalBounds().height) / 2);
                    if (text.getString().getSize() == 6)
                        timer = ElapsedTime;
                }
                window.draw(text);
            } else if (curStage == stages::HereWait) {
                window.clear(background);
                textColor += ChangeColor;
                text.setFillColor(textColor);
                window.clear(background);
                window.draw(text);
            } else if (curStage == stages::BlackHole) {
                window.clear(background);
                if (BlackCircle.getPosition().x != -1) {
                    if (BlackCircle.getRadius() < scw + sch) {
                        BlackCircle.setPosition(BlackCircle.getPosition() + sf::Vector2f{BlackCircle.getRadius(), BlackCircle.getRadius()});
                        BlackCircle.setRadius(BlackCircle.getRadius() * 1.02f + 1.f);
                        BlackCircle.setPosition(BlackCircle.getPosition() - sf::Vector2f{BlackCircle.getRadius(), BlackCircle.getRadius()});
                    } else timer = ElapsedTime;
                }
                window.draw(BlackCircle);
            } else if (curStage == stages::end1) {
                window.clear(background);
            } else if (curStage == stages::end2) {
                window.clear(background);
            }
        } else {
            timer = ElapsedTime;
            if (curStage == stages::intro) {
                                                                timer += sf::seconds(11);
                curStage = stages::HelloStranger;
                whoosh.play();
                machine.setVolume(50);
            } else if (curStage == stages::HelloStranger) {
                                                                timer += sf::seconds(25);
                curStage = stages::NobodyHere;
                seconds = int(ElapsedTime.asSeconds());
                text.setString("Здесь нет птиц, только Ты");
                text.setPosition((scw - text.getGlobalBounds().width) / 2,
                                (sch - text.getGlobalBounds().height) / 2);
                machine.setVolume(100);
            } else if (curStage == stages::NobodyHere) {
                                                                timer += sf::seconds(5);
                birds.stop();
                curStage = stages::You;
            } else if (curStage == stages::You) {
                                                                timer += sf::seconds(25);
                curStage = stages::YouHere;
                seconds = int(ElapsedTime.asSeconds());
                counter = 0;
            } else if (curStage == stages::YouHere) {
                                                                timer += sf::seconds(5);
                curStage = stages::YouHereWait;
                whoosh.stop();
                machine.stop();
            } else if (curStage == stages::YouHereWait) {
                                                                timer += sf::seconds(25);
                curStage = stages::Here;
                seconds = int(ElapsedTime.asSeconds());
                metronom.stop();
            } else if (curStage == stages::Here) {
                                                                timer += sf::seconds(5);
                curStage = stages::HereWait;
            } else if (curStage == stages::HereWait) {
                                                                timer += sf::seconds(60);
                curStage = stages::BlackHole;
                window.setMouseCursorVisible(true);
                BlackCircle.setFillColor(sf::Color::Black);
                BlackCircle.setRadius(0);
                BlackCircle.setPosition(-1, -1);
            } else if (curStage == stages::BlackHole) {
                                                                timer += sf::seconds(10);
                curStage = ((BlackCircle.getPosition().x == -1) ? stages::end1 : stages::end2);
                background = ((BlackCircle.getPosition().x == -1) ? sf::Color::White : sf::Color::Black);
            } else if (curStage == stages::end1) {

            } else if (curStage == stages::end2) {

            }
        }

        window.display();
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
            if (curStage == stages::BlackHole)
                if (event.type == sf::Event::MouseButtonPressed)
                    if (event.mouseButton.button == sf::Mouse::Button::Left && BlackCircle.getPosition().x == -1)
                        BlackCircle.setPosition((sf::Vector2f)(sf::Vector2i{event.mouseButton.x, event.mouseButton.y}));
        }
    }

    machine.stop(); whoosh.stop(); metronom.stop(); birds.stop();
    return 0;
}
