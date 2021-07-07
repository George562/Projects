#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
// -lsfml-graphics -lsfml-window -lsfml-system
using vi = std::vector<int>;
using vvi = std::vector<vi>;

int n = 256, size = 3, x, y;
int filling = 0, scs = n * size;
bool drawing = false, pause = true;

vvi dudes(n, vi(n, 0));
vvi change(n * n, vi(2, 0));

vvi colors = {
    {200, 200, 200}, {255,   0,   0},  // white, red
    {255, 255, 255}, {255, 100, 100},  // lwhite, lred
    {100, 100, 100}, {150,   0,   0}   // gray, bred
};

sf::RectangleShape rect1(sf::Vector2f(size, size));
sf::RectangleShape rect2(sf::Vector2f(size, size / 7));
sf::ConvexShape convex;
sf::Mouse Mouse;

sf::RenderWindow window(sf::VideoMode(scs, scs), "game in life");

int find(int x, int y);
void draw(int x, int y, int c);
void step();

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    convex.setPointCount(4);

    sf::Clock clock;
    sf::Time time = clock.getElapsedTime();

    while (window.isOpen()) {
        if (drawing) {
            sf::Vector2i mouse = Mouse.getPosition(window);
            x = mouse.x / size; y = mouse.y / size;
            if (0 <= x && x < n && 0 <= y && y < n)
                dudes[y][x] = filling;
        }

        if (clock.getElapsedTime() - time >= sf::milliseconds(50)) {
            if (!pause)
                step();

            window.clear();
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
                switch (event.key.code) {
                    case sf::Keyboard::Space: pause = !pause; break;
                    case sf::Keyboard::Tab: dudes.clear(); break;
                    case sf::Keyboard::Escape: window.close(); break;
                }

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

void step() {
    int x, y, count, cur = 0;
    for (y = 0; y < n; y++)
        for (x = 0; x < n; x++) {
            count = find(x, y);
            if (dudes[y][x] != 1 && count == 3) {
                change[cur][0] = y; change[cur++][1] = x;
            } else if (dudes[y][x] == 1 && count != 2 && count != 3) {
                change[cur][0] = y; change[cur++][1] = x;
            }
        }
    for (cur = cur - 1; cur >= 0; cur--) {
        y = change[cur][0]; x = change[cur][1];
        dudes[y][x] = (dudes[y][x] + 1) % 2;
        }
}

int find(int x, int y) {
    int sum = -dudes[y][x];
    for (int dy = y - 1; dy < y + 2; dy++)
        for (int dx = x - 1; dx < x + 2; dx++)
            sum += dudes[(n + (dy % n)) % n][(n + (dx % n)) % n];
    return sum;
}

void draw(int x, int y, int c) {
    if (c == 0) return;
    using namespace sf;
    x *= size; y *= size;

    rect1.setFillColor(Color(colors[c][0], colors[c][1], colors[c][2]));
    rect1.setPosition(x, y);
    window.draw(rect1);

    if (n <= 128) {
        c += 2;
        rect2.setFillColor(Color(colors[c][0], colors[c][1], colors[c][2]));
        rect2.setPosition(x, y);
        window.draw(rect2);

        c += 2;
        convex.setFillColor(Color(colors[c][0], colors[c][1], colors[c][2]));
        convex.setPoint(0, Vector2f(x, y));
        convex.setPoint(1, Vector2f(x, y + size));
        convex.setPoint(2, Vector2f(x + size / 7, y + size));
        convex.setPoint(3, Vector2f(x + size / 7, y + size / 7));
        window.draw(convex);
    }
}
