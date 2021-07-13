#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
// -lsfml-graphics -lsfml-window -lsfml-system
struct Dude { int x, y; };
bool operator==(Dude& a, Dude& b) { return a.x == b.x && a.y == b.y; }
bool operator!=(Dude& a, Dude& b) { return a.x != b.x || a.y != b.y; }
bool operator<(Dude a, Dude b) { if (a.y == b.y) return a.x < b.x; return a.y < b.y; }
Dude operator+(Dude& a, Dude& b) { return Dude{a.x + b.x, a.y + b.y}; }

int scw = sf::VideoMode::getDesktopMode().width;
int sch = sf::VideoMode::getDesktopMode().height;
int size = 2, filling = 0, left = 0, top = 0, x, y;
int to_burn = 8, to_alive = 12, set_x = 3 * scw / 8, set_y = 7 * sch / 16;
bool drawing = false, pause = true, settings = false;
std::vector<Dude> dudes(0), new_dudes(0);
std::vector<int> red = {255,   0,   0}, green = {   0, 255,   0}, white = {200, 200, 200}, changes(0);
std::vector<Dude> dirs = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
Dude bufferDude;

sf::RectangleShape rect1(sf::Vector2f(size, size));
sf::RectangleShape menu_plato(sf::Vector2f(scw / 4, sch / 8));
sf::CircleShape circle;
sf::Mouse Mouse;
sf::Vector2i mouse;
sf::RenderWindow window(sf::VideoMode(scw, sch), "infinity", sf::Style::Fullscreen);
sf::Font font;
sf::Text text, set_text;
std::string word;
sf::Event event;

bool into(std::vector<Dude>& arr, Dude d) {
    int i = std::lower_bound(arr.begin(), arr.end(), d) - arr.begin();
    return !(arr.empty() || (arr[i] != d && (i + 1 < arr.size() || arr[i + 1] != d)));
}
void add(std::vector<Dude>&, Dude);
void del(Dude);
void draw();
void step();
void stepfor(int, int);

int main() {
    std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    text.setFont(font);
    text.setCharacterSize(18);
    text.setFillColor(sf::Color::White);
    text.setPosition(0, 0);
    
    set_text.setFont(font);
    set_text.setCharacterSize(20);
    set_text.setFillColor(sf::Color::White);
    set_text.setPosition(set_x + scw / 64, set_y + sch / 64);
    set_text.setString("Burn\n\nAlive\n\t\t\t0\t1\t2\t3\t4\t5\t6\t7\t8");
    
    menu_plato.setFillColor(sf::Color(20, 20, 20));
    menu_plato.setOutlineColor(sf::Color(white[0], white[1], white[2]));
    menu_plato.setOutlineThickness(5);
    menu_plato.setPosition(set_x, set_y);

    circle.setRadius(10);
    circle.setOutlineColor(sf::Color(0, 0, 0));
    circle.setOutlineThickness(3);

    rect1.setFillColor(sf::Color(red[0], red[1], red[2]));

    sf::Clock clock;
    sf::Time time = clock.getElapsedTime();

    while (window.isOpen()) {
        if (drawing && !settings) {
            mouse = Mouse.getPosition(window);
            x = mouse.x; y = mouse.y;
            if (0 <= x && x < scw && 0 <= y && y < sch) {
                x = (mouse.x + left) / size; y = (mouse.y + top) / size;
                if (filling) add(dudes, Dude({x, y}));
                else del(Dude{x, y});
            }
        }
        
        if (dudes.size() > 2000 && !pause) step();

        if (clock.getElapsedTime() - time >= sf::milliseconds(50)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) top -= size;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) left -= size;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) top += size;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) left += size;

            if (dudes.size() <= 2000 && !pause) step();
            
            window.clear();
            draw();
            word = "x: " + std::to_string(left) +
                    "\ny: " + std::to_string(top) +
                    "\nx" + std::to_string(size) +
                    "\ncount of dudes: " + std::to_string(dudes.size()) +
                    "\n" + std::to_string((clock.getElapsedTime() - time).asSeconds());
            text.setString(word);
            window.draw(text);
            window.display();
            time = clock.getElapsedTime();
        }

        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::KeyPressed)
                switch (event.key.code) {
                    case sf::Keyboard::Space: pause = !pause; break;
                    case sf::Keyboard::Tab: settings = !settings; break;
                    case sf::Keyboard::Escape: window.close(); break;
                }
            else if (event.type == sf::Event::MouseButtonReleased) drawing = false;
            else if (event.type == sf::Event::MouseButtonPressed) {
                drawing = true;
                if (event.mouseButton.button != sf::Mouse::Button::Middle)
                    filling = 1 - event.mouseButton.button;
                
                if (settings) {
                    mouse = Mouse.getPosition(window);
                    int rad = circle.getRadius();
                    for (int i = 0; i < 9; i++) {
                        int circle_x = set_x + scw / 32 + 35 * (1 + i) + 2 + rad;
                        if (std::hypot(circle_x - mouse.x, set_y + sch / 64 + 3 + rad - mouse.y) <= rad)
                            to_burn = (to_burn & (1 << i)) ? to_burn - (1 << i) : to_burn + (1 << i);
                        if (std::hypot(circle_x - mouse.x, set_y + sch / 64 + 50 + rad - mouse.y) <= rad)
                            to_alive = (to_alive & (1 << i)) ? to_alive - (1 << i) : to_alive + (1 << i);
                    }
                }
            } else if (event.type == sf::Event::MouseWheelScrolled)
                if (event.mouseWheelScroll.delta != 0) {
                    mouse = Mouse.getPosition(window);
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

void draw() {
    int dx, dy;
    for (int i = 0; i < dudes.size(); i++) {
        dx = dudes[i].x * size; dy = dudes[i].y * size;
        if (left <= dx && dx < left + scw && top <= dy && dy < top + sch) {
            rect1.setPosition(dx - left, dy - top);
            window.draw(rect1);
        }
    }
    if (settings) {
        window.draw(menu_plato);
        window.draw(set_text);
        for (int i = 0; i < 9; i++) {
            if (to_burn & (1 << i)) circle.setFillColor(sf::Color(green[0], green[1], green[2]));
            else circle.setFillColor(sf::Color(red[0], red[1], red[2]));
            circle.setPosition(set_x + scw / 32 + 35 * (1 + i) + 2, set_y + sch / 64 + 3);
            window.draw(circle);

            if (to_alive & (1 << i)) circle.setFillColor(sf::Color(green[0], green[1], green[2]));
            else circle.setFillColor(sf::Color(red[0], red[1], red[2]));
            circle.setPosition(set_x + scw / 32 + 35 * (1 + i) + 2, set_y + sch / 64 + 50);
            window.draw(circle);
        }
    }
}

void step() {
    changes.clear(); new_dudes.clear();
    int count, size = dudes.size();
    for (int i = size - 1; i >= 0; i--) {
        count = 1;
        for (int j = 0; j < 8; j++)
            if (into(dudes, dudes[i] + dirs[j])) count <<= 1;
            else stepfor(i, j);
        if (!(count & to_alive)) changes.push_back(i);
    }
    for (int i = 0; i < changes.size(); i++, size--)
        std::swap(dudes[changes[i]], dudes[size - 1]);
    dudes.resize(size);
    std::sort(new_dudes.begin(), new_dudes.end());
    for (int i = 0; i < new_dudes.size(); i++)
        if (i == 0 || new_dudes[i - 1] < new_dudes[i]) dudes.push_back(new_dudes[i]);
    std::sort(dudes.begin(), dudes.end());
}

void stepfor(int i, int j) {
    int count = 2;
    bufferDude = dudes[i] + dirs[j];
    for (int k = (j + 5) % 8; k != (j + 4) % 8; k = ++k % 8)
        if (into(dudes, bufferDude + dirs[k])) count <<= 1;
    if (count & to_burn) new_dudes.push_back(bufferDude);
}
