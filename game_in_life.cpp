#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
// -lsfml-graphics -lsfml-window -lsfml-system

int n = 64, size = 14, x, y;

std::vector<std::vector<int>> dudes(n, std::vector<int>(n, 0));
std::vector<std::vector<int>> new_dudes(n, std::vector<int>(n, 0));

std::vector<std::vector<int>> colors = {
    {200, 200, 200}, {255,   0,   0},  // white, red
    {255, 255, 255}, {255, 100, 100},  // lwhite, lred
    {100, 100, 100}, {150,   0,   0}   // gray, bred
};

sf::RectangleShape rect1(sf::Vector2f(size, size));
sf::RectangleShape rect2(sf::Vector2f(size, size / 7));
sf::ConvexShape convex;

std::vector<sf::RectangleShape> rects1(n * n, sf::RectangleShape(sf::Vector2f(size, size)));
std::vector<sf::RectangleShape> rects2(n * n, sf::RectangleShape(sf::Vector2f(size, size / 7)));
std::vector<sf::ConvexShape> convexes(n * n);

sf::RenderWindow window(sf::VideoMode(n * size, n * size), "game in life");

// sf::Texture alive_texture, dead_texture;
// sf::Sprite alive, dead;

int find(int x, int y);

void draw(int x, int y, int c);

void play();

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    // window.setVerticalSyncEnabled(true);
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            rects1[y * n + x].setPosition(x * size, y * size);
            rects2[y * n + x].setPosition(x * size, y * size);

            convexes[y * n + x].setPointCount(4);
            convexes[y * n + x].setPoint(0, sf::Vector2f(x * size, y * size));
            convexes[y * n + x].setPoint(1, sf::Vector2f(x * size, y * size + size));
            convexes[y * n + x].setPoint(2, sf::Vector2f(x * size + size / 7, y * size + size));
            convexes[y * n + x].setPoint(3, sf::Vector2f(x * size + size / 7, y * size + size / 7));
            draw(x, y, 0);
        }
    convex.setPointCount(4);
    // alive_texture.loadFromFile("alive.png");
    // dead_texture.loadFromFile("dead.png");
    // alive.setTexture(alive_texture); dead.setTexture(dead_texture);
    // alive.setTextureRect(sf::IntRect(10, 10, 32, 32));
    // alive.setScale(size, size); dead.setScale(size, size);

    bool drawing = false, pause = true;
    int filling = 0;

    sf::Clock clock;
    sf::Time time = clock.getElapsedTime();

    // for (int y = 0; y < n; y++)
    //     for (int x = 0; x < n; x++)
    //         draw(x, y, 0);

    while (window.isOpen()) {
        if (drawing) {
            sf::Vector2i mouse = sf::Mouse::getPosition(window);
            x = mouse.x; y = mouse.y;
            if (0 <= x && x < size * n && 0 <= y && y < size * n) {
                draw(x / size, y / size, filling);
                new_dudes[y / size][x / size] = filling;
                dudes[y / size][x / size] = filling;
            }
        }

        if (clock.getElapsedTime() - time >= sf::milliseconds(40)) {
            if (!pause)
                play();
            
            for (int y = 0; y < n; y++)
                for (int x = 0; x < n; x++)
                    draw(x, y, dudes[y][x]);

            window.display();
            time = clock.getElapsedTime();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
                if (event.key.code == 57)
                    pause = !pause;

            if (event.type == sf::Event::MouseButtonReleased)
                drawing = false;

            if (event.type == sf::Event::MouseButtonPressed) {
                drawing = true;
                if (event.mouseButton.button != 3)
                    filling = 1 - event.mouseButton.button;
            }
        }
    }
    return 0;
}

void play() {
    int count;
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            count = find(x, y);
            if (count == 3) {
                new_dudes[y][x] = 1;
            } else if (count != 2) {
                new_dudes[y][x] = 0;
            }
        }
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++)
            dudes[y][x] = new_dudes[y][x];
}

int find(int x, int y) {
    int sum = -dudes[y][x];
    for (int dy = y - 1; dy < y + 2; dy++)
        for (int dx = x - 1; dx < x + 2; dx++)
            sum += dudes[(n + (dy % n)) % n][(n + (dx % n)) % n];
    return sum;
}

void draw(int x, int y, int c) {
    rects1[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    rects1[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    window.draw(rects1[y * n + x]);

    c += 2;
    rects2[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    rects2[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    window.draw(rects2[y * n + x]);

    c += 2;
    convexes[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    convexes[y * n + x].setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    window.draw(convexes[y * n + x]);
    // x *= size; y *= size;
    // if (c) {alive.setPosition(sf::Vector2f(x, y)); window.draw(alive);}
    // else {dead.setPosition(sf::Vector2f(x, y)); window.draw(dead);}

    // rect1.setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    // rect1.setPosition(x, y);
    // window.draw(rect1);

    // c += 2;
    // rect2.setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    // rect2.setPosition(x, y);
    // window.draw(rect2);

    // c += 2;
    // convex.setFillColor(sf::Color(colors[c][0], colors[c][1], colors[c][2]));
    // convex.setPoint(0, sf::Vector2f(x, y));
    // convex.setPoint(1, sf::Vector2f(x, y + size));
    // convex.setPoint(2, sf::Vector2f(x + size / 7, y + size));
    // convex.setPoint(3, sf::Vector2f(x + size / 7, y + size / 7));
    // window.draw(convex);
}
