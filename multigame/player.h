#include "creature.h"

////////////////////////////////////////////////////////////
// Class
////////////////////////////////////////////////////////////

class Player : public Creature {
public:
    sf::RectangleShape ManaRect[3], HpRect[3];
    PlaccedText HpText, ManaText;
    bool ShiftPressed;

    Player();
    void draw(sf::RenderWindow&);
    void interface(sf::RenderWindow&);
    void move(const vvr&, int&);
    void update(vB&);
    void update(sf::Event&, bool&);
    void ChangeWeapon(int);
    void ChangeWeapon(Weapon*);
};

////////////////////////////////////////////////////////////
// Realization
////////////////////////////////////////////////////////////

Player::Player() : Creature() {
    Health = {0, 10, 10};
    Mana = {0, 75, 75}; ManaRecovery = 0.5;
    Armor = {0, 0, 0};
    Money = 0;
    radius = 60;
    Width = Height = radius * 2;
    PosX = 0; PosY = 0; Velocity = {{-6, -6}, {6, 6}, {0, 0}}, Acceleration = 0.6;
    ShiftPressed = false;
    LastCheck = sf::Clock().getElapsedTime();
    circle.setRadius(radius);
    // circle.setPointCount(4);
    circle.setFillColor(sf::Color(0, 180, 0));
    circle.setOutlineColor(sf::Color(100, 50, 50));
    circle.setOutlineThickness(Velocity.cur.x);
    SetTexture("sources/Player");

    HpRect[0].setSize({360, 60});
    HpRect[1].setSize({350, 50});
    HpRect[0].setPosition(scw - HpRect[0].getSize().x - 10, 20);
    HpRect[1].setPosition(HpRect[0].getPosition().x + 5, HpRect[0].getPosition().y + 5);
    HpRect[0].setFillColor(sf::Color(255, 255, 255, 160));
    HpRect[1].setFillColor(sf::Color(192, 0, 0, 160));

    HpRect[2] = HpRect[1];
    HpRect[2].setFillColor(sf::Color(32, 32, 32, 160));

    ManaRect[0].setSize({240, 50});
    ManaRect[1].setSize({230, 40});
    ManaRect[0].setPosition(scw - ManaRect[0].getSize().x - 10, HpRect[0].getPosition().y + HpRect[0].getSize().y - 5);
    ManaRect[1].setPosition(ManaRect[0].getPosition().x + 5, ManaRect[0].getPosition().y + 5);
    ManaRect[0].setFillColor(sf::Color(255, 255, 255, 160));
    ManaRect[1].setFillColor(sf::Color(0, 0, 192, 160));

    ManaRect[2] = ManaRect[1];
    ManaRect[2].setFillColor(sf::Color(32, 32, 32, 160));
}

void Player::interface(sf::RenderWindow& window) {
    HpText.setText(std::to_string((int)Health.cur));
    HpText.setPosition(HpRect[0].getPosition().x + HpRect[0].getSize().x / 2 - HpText.Width  / 2,
                       HpRect[0].getPosition().y + HpRect[0].getSize().y / 2 - HpText.Height / 2);
    HpRect[1].setScale(Health.filling(), 1);
    ManaText.setText(std::to_string((int)Mana.cur));
    ManaText.setPosition(ManaRect[0].getPosition().x + ManaRect[0].getSize().x / 2 - ManaText.Width  / 2,
                       ManaRect[0].getPosition().y + ManaRect[0].getSize().y / 2 - ManaText.Height / 2);
    ManaRect[1].setScale(Mana.filling(), 1);
    window.draw(HpRect[0]);
    window.draw(HpRect[2]);
    window.draw(HpRect[1]);
    window.draw(ManaRect[0]);
    window.draw(ManaRect[2]);
    window.draw(ManaRect[1]);
    HpText.draw(window);
    ManaText.draw(window);
    CurWeapon->interface(window);
}

void Player::draw(sf::RenderWindow& window) {
    if (rect.getGlobalBounds().width == 0) {
        circle.setPosition(PosX - Camera->x, PosY - Camera->y);
        window.draw(circle);
    } else {
        rect.setPosition(PosX - Camera->x, PosY - Camera->y);
        window.draw(rect);
    }
}

void Player::move(const vvr& walls, int& size) {
    int N = walls.size(), M = walls[0].size(), y = (int(PosY) / size) * 2, x = (int(PosX) / size) * 2;

    if (!(0 <= y && y < N && 0 <= x && x < M)) return;

    std::map<char, bool> PressedKeys = {
        {'w', sf::Keyboard::isKeyPressed(sf::Keyboard::W)},
        {'a', sf::Keyboard::isKeyPressed(sf::Keyboard::A)},
        {'s', sf::Keyboard::isKeyPressed(sf::Keyboard::S)},
        {'d', sf::Keyboard::isKeyPressed(sf::Keyboard::D)}
    };

    if (PressedKeys['w'] || PressedKeys['s']) {
        Velocity.cur.y += Acceleration * ((PressedKeys['s']) ? 1 : -1);
        if (Velocity.fromTop().y < 0 || Velocity.toBottom().y < 0)
            Velocity.cur.y -= Acceleration * ((PressedKeys['s']) ? 1 : -1);
        if      (Velocity.fromTop().y  < 0) Velocity.cur.y -= Acceleration;
        else if (Velocity.toBottom().y < 0) Velocity.cur.y += Acceleration;
    } else {
        if (std::abs(Velocity.cur.y) <= Acceleration)
            Velocity.cur.y = 0;
        else if (Velocity.cur.y != 0)
            Velocity.cur.y -= Acceleration * ((Velocity.cur.y > 0) ? 1 : -1);
    }
    
    if (PressedKeys['a'] || PressedKeys['d']) {
        Velocity.cur.x += Acceleration * ((PressedKeys['d']) ? 1 : -1);
        if (Velocity.fromTop().x < 0 || Velocity.toBottom().x < 0)
            Velocity.cur.x -= Acceleration * ((PressedKeys['d']) ? 1 : -1);
        if      (Velocity.fromTop().x  < 0) Velocity.cur.x -= Acceleration;
        else if (Velocity.toBottom().x < 0) Velocity.cur.x += Acceleration;
    } else {
        if (std::abs(Velocity.cur.x) <= Acceleration)
            Velocity.cur.x = 0;
        else if (Velocity.cur.x != 0)
            Velocity.cur.x -= Acceleration * ((Velocity.cur.x > 0) ? 1 : -1);
    }

    sf::Vector2i tempv = WillCollisionWithWalls(walls, size, PosX, PosY, Width, Height, Velocity.cur.x, Velocity.cur.y);

    if (tempv.x == 1) PosX += Velocity.cur.x;
    else Velocity.cur.x = 0;
    if (tempv.y == 1) PosY += Velocity.cur.y;
    else Velocity.cur.y = 0;
}

void Player::update(vB& Bullets) {
    Mana += ManaRecovery * (Clock->getElapsedTime() - LastCheck).asSeconds();
    if (CurWeapon != nullptr) {
        CurWeapon->Update(Bullets, *this, Clock, Camera);
    }
    LastCheck = Clock->getElapsedTime();
}

void Player::update(sf::Event& event, bool& MiniMapActivated) {
    if (CurWeapon != nullptr) {
        if (!MiniMapActivated)
            CurWeapon->Update(event);
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::R)
            CurWeapon->Reload(Mana);
        else if (event.key.code == sf::Keyboard::LShift && !ShiftPressed) {
            Velocity.top    *= 2.f;
            Velocity.bottom *= 2.f;
            ShiftPressed = true;
        }
    } else if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::LShift) {
            Velocity.top    /= 2.f;
            Velocity.bottom /= 2.f;
            ShiftPressed = false;
        }
    }
}

void Player::ChangeWeapon(int to) {
    CurWeapon = ((to == sf::Keyboard::Num1) ? FirstWeapon : SecondWeapon);
}

void Player::ChangeWeapon(Weapon* to) {
    CurWeapon = to;
}

sf::Packet& operator<<(sf::Packet& packet, Player& a) { return packet << a.PosX << a.PosY; }
sf::Packet& operator>>(sf::Packet& packet, Player& a) { return packet >> a.PosX >> a.PosY; }
