#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
// -lsfml-graphics -lsfml-window -lsfml-system
struct Dude { int x, y; };
bool operator==(Dude a, Dude b) { return a.x == b.x && a.y == b.y; }
bool operator!=(Dude a, Dude b) { return a.x != b.x || a.y != b.y; }
bool operator<(Dude a, Dude b) { if (a.y == b.y) return a.x < b.x; return a.y < b.y; }
Dude operator+(Dude a, Dude b) { return Dude{a.x + b.x, a.y + b.y}; }

using vi = std::vector<int>;
using pii = std::pair<int, int>;

int size = 2, filling = 0, left = 0, top = 0, x, y;
int scw = sf::VideoMode::getDesktopMode().width;
int sch = sf::VideoMode::getDesktopMode().height;
bool drawing = false, pause = true;
std::vector<Dude> dudes(0), new_dudes(0);
vi red = {255,   0,   0}, white = {200, 200, 200}, changes(0);
std::vector<Dude> dirs= {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

sf::RectangleShape rect1(sf::Vector2f(size, size));
sf::Mouse Mouse;
sf::RenderWindow window(sf::VideoMode(scw, sch), "infinity", sf::Style::Fullscreen);

bool into(std::vector<Dude> arr, Dude d) {
    int i = std::lower_bound(arr.begin(), arr.end(), d) - arr.begin();
    return !(arr.empty() || (arr[i] != d && (i + 1 < arr.size() || arr[i + 1] != d)));
}
void add(std::vector<Dude>&, Dude);
void del(Dude);
void draw(Dude);
void step();
void stepfor(Dude);
int find(int, int);

int main() {
    // std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    sf::Font font;
    font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition(0, 0);
    std::string word = "left: " + std::to_string(left) + "\ntop: " + std::to_string(top);
    text.setString(word);

    sf::Clock clock;
    sf::Time time = clock.getElapsedTime();

    while (window.isOpen()) {
        if (drawing) {
            sf::Vector2i mouse = Mouse.getPosition(window);
            x = mouse.x; y = mouse.y;
            if (0 <= x && x < scw && 0 <= y && y < sch) {
                x = (mouse.x + left) / size; y = (mouse.y + top) / size;
                if (filling) add(dudes, Dude({x, y}));
                else del(Dude{x, y});
            }
        }

        if (clock.getElapsedTime() - time >= sf::milliseconds(50)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) top -= size * 4;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) left -= size * 4;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) top += size * 4;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) left += size * 4;
            if (!pause) step();

            window.clear();
            for (Dude dude: dudes) draw(dude);
            word = "x: " + std::to_string(left) + 
                    "\ny: " + std::to_string(top) + 
                    "\nx" + std::to_string(size);
            text.setString(word);
            window.draw(text);
            window.display();
            time = clock.getElapsedTime();
        }

        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::KeyPressed)
                switch (event.key.code) {
                    case sf::Keyboard::Space: pause = !pause; break;
                    case sf::Keyboard::Tab: dudes.clear(); left = top = 0; break;
                    case sf::Keyboard::Escape: window.close(); break;
                }
            else if (event.type == sf::Event::MouseButtonReleased) drawing = false;
            else if (event.type == sf::Event::MouseButtonPressed) {
                drawing = true;
                if (event.mouseButton.button != 3)
                    filling = 1 - event.mouseButton.button;
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
                if (event.mouseWheelScroll.delta != 0) {
                    sf::Vector2i mouse = Mouse.getPosition(window);
                    if (event.mouseWheelScroll.delta == 1) {
                        left = left * 2 + mouse.x;
                        top = top * 2 + mouse.y;
                        size *= 2;
                    } else if (size > 1) {
                        left = left / 2 - mouse.x / 2;
                        top = top / 2 - mouse.y / 2;
                        size /= 2;
                    }
                    rect1.setSize(sf::Vector2f(size, size));
                }
    }
    std::cout << "count of dudes = " << dudes.size() << '\n';
    return 0;
}

void add(std::vector<Dude>& arr, Dude d) {
    int i = std::lower_bound(arr.begin(), arr.end(), d) - arr.begin();
    if (arr.empty() || (arr[i] != d && (i + 1 < arr.size() || arr[i + 1] != d))) {
        arr.push_back(d);
        for (int j = arr.size() - 1; j > i; j--) std::swap(arr[j], arr[j - 1]);
    }
}

void del(Dude d) {
    if (dudes.empty()) return;
    int i = std::lower_bound(dudes.begin(), dudes.end(), d) - dudes.begin();
    if (i >= dudes.size() || (dudes[i] != d && (i + 1 < dudes.size() || dudes[i + 1] != d))) return;
    int j = dudes.size() - 1;
    std::swap(dudes[i], dudes[j]);
    for (; i < j - 1; i++) std::swap(dudes[i], dudes[i + 1]);
    dudes.pop_back();
}

void draw(Dude d) {
    int dx = d.x * size, dy = d.y * size;
    if (left <= dx && dx < left + scw && top <= dy && dy < top + sch) {
        rect1.setFillColor(sf::Color(red[0], red[1], red[2]));
        rect1.setPosition(dx - left, dy - top);
        window.draw(rect1);
    }
}

void step() {
    changes.clear(); new_dudes.clear();
    int count, size = dudes.size();
    for (int i = size - 1; i >= 0; i--) {
        count = 0;
        for (Dude ch: dirs) {
            Dude d = dudes[i] + ch;
            if (into(dudes, d)) count++;
            else if (!into(new_dudes, d)) stepfor(d);
        }
        if (count != 2 && count != 3) changes.push_back(i);
    }
    for (int i = 0; i < changes.size(); i++) {
        std::swap(dudes[changes[i]], dudes[dudes.size() - 1]);
        dudes.pop_back();
    }
    for (Dude d: new_dudes) dudes.push_back(d);
    std::sort(dudes.begin(), dudes.end());
}

void stepfor(Dude d) {
    int count = 0;
    for (Dude ch: dirs) if (into(dudes, d + ch)) count++;
    if (count == 3) add(new_dudes, d);
}
