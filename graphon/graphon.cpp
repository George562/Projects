#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <string>
// -lsfml-graphics -lsfml-window -lsfml-system
using str = std::string;
using mapss = std::map<str, str>;
using vs = std::vector<str>;
using vvs = std::vector<vs>;

int scw = 1000, sch = 600, textX = 50, textY = 50;
sf::RenderWindow window(sf::VideoMode(scw, sch), "graphon");
sf::Event event;
sf::Font font;
sf::Text text, tabletext;
sf::RectangleShape rectangle;

void draw(sf::Text&, vvs&);
void work(str&, vvs&);
str convert(str&);
void getData(str&, vs&, mapss&, vs&);
str normalize(str, vs&);
void makeTable(vs&, mapss&, vs&, vvs&);
void showTable(vvs&);
str operator*(str a, str b) {
    if (a == "e" && b != "") return b;
    if (b == "e" && a != "") return a;
    return a + b;
}
void operator*=(str& a, str& b) { a = a * b; }
bool operator<(str& a, str& b) {
    // if (a.size() < b.size()) return true;        WHAT THE HELL
    if (a == "e") return true;
    if (b == "e") return false;
    if (a.size() == b.size()) {
        for (int i = 0; i < a.size(); i++)
            if (a[i] < b[i]) return true;
            else if (a[i] > b[i]) return false;
    }
    return false;
}
bool into(str& a, vs& b) { for (str x: b) if (a == x) return true; return false; }

int main() {
    font.loadFromFile("arial.ttf");
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::Black);
    text.setPosition(textX, textY);

    tabletext.setFont(font);
    tabletext.setCharacterSize(20);
    tabletext.setFillColor(sf::Color::Black);

    rectangle.setPosition(textX, textY);
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setOutlineThickness(2);
    rectangle.setSize(sf::Vector2f(scw - 100, 30));

    str word;
    text.setString(word);
    vvs table(0);

    while (window.isOpen()) {
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::KeyPressed) {
                // std::cout << event.key.code << '\n';
                if (event.key.code == 59) {
                    if (word.size() != 0)
                        if (event.key.control) word.clear();
                        else word.pop_back();
                } else if (0 <= event.key.code && event.key.code < 26)
                    word.push_back('a' + event.key.code);
                else if (27 <= event.key.code && event.key.code < 36)
                    word.push_back('0' + event.key.code - 26);
                else if (event.key.code == 36) window.close();
                else if (event.key.code == 55) word.push_back('=');
                else if (event.key.code == 49) word.push_back(',');
                else if (event.key.code == 50) word.push_back('.');
                else if (event.key.code == 53 && event.key.shift)
                    word.push_back('|');
                else if (event.key.code == 56 && event.key.shift)
                    word.push_back('_');
                else if (event.key.code == 56) word.push_back('-');
                else if (event.key.code == 58)
                    work(word, table);
                text.setString(word);
            }
        draw(text, table);
    }
    return 0;
}

void draw(sf::Text& text, vvs& table) {
    window.clear(sf::Color::White);
    window.draw(rectangle);
    window.draw(text);
    showTable(table);
    window.display();
}

void showTable(vvs& table) {
    int x = 50, y = 100, dx = 90, dy = 30, Tlen = table.size();
    sf::RectangleShape Wline(sf::Vector2f(dx * Tlen, 3));
    sf::RectangleShape Hline(sf::Vector2f(3, dy * Tlen));
    Hline.setFillColor(sf::Color::Black); Wline.setFillColor(sf::Color::Black);
    for (int i = 0; i <= Tlen; i++) {
        Wline.setPosition(sf::Vector2f(x, y + dy * i));
        window.draw(Wline);
        Hline.setPosition(sf::Vector2f(x + dx * i, y));
        window.draw(Hline);
    }
    for (int i = 0; i < Tlen; i++)
        for (int j = 0; j < Tlen; j++) {
            tabletext.setString(table[i][j]);
            tabletext.setPosition(x + dx * j + 5, y + dy * i + 5);
            window.draw(tabletext);
        }
}

str convert(str& w) {
    str buf; buf.clear();
    for (char x: w)
        if ('1' <= x && x <= '9')
            for (int i = x - '1'; i > 0; i--)
                buf.push_back(buf[buf.size() - 1]);
        else buf.push_back(x);
    return buf;
}

void getData(str& word, vs& letters, mapss& rules, vs& rules_list) {
    int i;
    str first, second;
    for (i = 0; word[i] != '|' && i < word.size(); i++)
        if ('a' <= word[i] && word[i] <= 'z') letters.push_back(str(1, word[i]));
    while (i < word.size()) {
        first.clear(); second.clear();
        for (i++; word[i] != '='; i++) first.push_back(word[i]);
        for (i++; word[i] != ',' && i < word.size(); i++) second.push_back(word[i]);
        first = convert(first); second = convert(second);
        rules[first] = second; rules[second] = first;
        if (!(first < second)) {
            rules_list.push_back(first); rules_list.push_back(second);
        } else {
            rules_list.push_back(second); rules_list.push_back(first);
        }
    }
}

str normalize(str w, vs& rules_list) {
    bool change = true;
    while (change) {
        change = false;
        for (int i = 0, n; i < rules_list.size(); i += 2) {
            n = w.find(rules_list[i]);
            if (0 <= n && n < w.size() && rules_list[i + 1] == "e") {
                change = true;
                w = w.substr(0, n) * "e" * w.substr(n + rules_list[i].size());
                i = 0;
            }
        }
        for (int i = 0, n; i < rules_list.size(); i += 2) {
            n = w.find(rules_list[i]);
            if (0 <= n && n < w.size()) {
                change = true;
                w = w.substr(0, n) * rules_list[i + 1] * w.substr(n + rules_list[i].size());
            }
        }
    }
    return w;
}

void makeTable(vs& letters, mapss& rules, vs& rules_list, vvs& table) {
    str cur, buf;
    for (int i = 1; i < letters.size() && i <= 8; i++) {
        cur = letters[i];
        table[0].push_back(cur);
        vs line(1, cur);
        for (int j = 1; j < table.size(); j++) {
            buf = normalize(table[0][j] * cur, rules_list);
            table[j].push_back(buf);
            if (!into(buf, letters)) letters.push_back(buf);
        }
        for (int j = 1; j < table[0].size(); j++) {
            buf = normalize(cur * table[0][j], rules_list);
            line.push_back(buf);
            if (!into(buf, letters)) letters.push_back(buf);
        }
        table.push_back(line);
    }
}

void work(str& word, vvs& table) {
    mapss rules;
    vs rules_list(0), letters(0); letters.push_back("e");
    std::ifstream data(word);
    if (data.is_open()) data >> word;
    getData(word, letters, rules, rules_list);
    table.clear();
    table.push_back(vs{" ", "e"}); table.push_back(vs{"e", "e"});
    makeTable(letters, rules, rules_list, table);
}
