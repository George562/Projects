#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <string>
#include <iomanip>
// -lsfml-graphics -lsfml-window -lsfml-system
using namespace std;
using str = string;
using mapss = map<str, str>;
using vs = vector<str>;
using vvs = vector<vs>;

int scw = 1000, sch = 600, textX = 50, textY = 50;

void draw(vvs&);
void work(str&, vvs&);
str convert(str&);
void getData(str&, vs&, mapss&, vs&);
str normalize(str, vs&);
void makeTable(vs&, mapss&, vs&, vvs&);
str operator*(str a, str b) {
    if (a == "e" && b != "") return b;
    if (b == "e" && a != "") return a;
    return a + b;
}
void operator*=(str& a, str& b) { a = a * b; }
bool operator<(str& a, str& b) {
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
    str word;
    vvs table(0);
    cin >> word;
    work(word, table);
    draw(table);
    getchar();
    return 0;
}

void draw(vvs& table) {
    ofstream file("output.txt");
    for (int i = 0; i < 9 * table.size(); i++) file << '_';
    for (int i = 0; i < table.size(); i++) {
        file << "\n|";
        for (int j = 0; j < table.size(); j++)
            file << setw(8) << left << table[i][j] << "|";
        file << '\n';
        for (int i = 0; i < 9 * table.size(); i++) file << '-';
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
    for (int i = 1; i < letters.size() && i <= 100; i++) {
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
    ifstream data(word);
    if (data.is_open()) data >> word;
    getData(word, letters, rules, rules_list);
    table.clear();
    table.push_back(vs{" ", "e"}); table.push_back(vs{"e", "e"});
    makeTable(letters, rules, rules_list, table);
}
