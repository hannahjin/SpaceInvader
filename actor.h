#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class StudentWorld;

const int IID_ALIEN = -1;
const int DEFAULT_PLAYER_ENERGY = 50;
const int NACHLING_STARTING_ENERGY_FACTOR = 5;
const int WEALTHY_NACHLING_STARTING_ENERGY_FACTOR = 8;
const int SMALLBOT_STARTING_ENERGY_FACTOR = 12;
const int BULLET_DAMAGE = 2;
const int TORPEDO_DAMAGE = 8;
const int LEFT = 0;
const int RIGHT = 1;
const int GOODIE_SCORE = 5000;
const int BASE_CHANCE_OF_FIRING = 10;
const int CHANCE_TRANS_STATE2 = 20;
const int COLLIDE_PLAYERSHIP = 100000;
const int COLLIDE_ALIEN_DAMAGE = 15;
const int COLLIDE_BULLET_DAMAGE = 2;
const int COLLIDE_TORPEDO_DAMAGE = 8;
const int MALFUNC_TICKS = 30;
const int TORPEDO_GOODIE_PACKAGE_NUM = 5;

// Students:  Add code to this file, actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor: public GraphObject {
public:
    Actor(int imageID, int startX, int startY, StudentWorld* sw);
    virtual ~Actor() {}
    virtual void doSomething() = 0;
    virtual bool alive();
    void kill() {m_alive = false; setVisible(false);}
    StudentWorld* getWorld() {return m_studentWorld;}
    //    virtual void damage(int injurypoints);
private:
    bool m_alive;
    StudentWorld* m_studentWorld;
};

class Star: public Actor{
public:
    Star(StudentWorld* sw);
    virtual ~Star() {}
    virtual void doSomething();
};

class Ship: public Actor{
public:
    Ship(int imageID, int startX, int startY, StudentWorld* sw);
    virtual ~Ship() {}
    int getEnergy() {return m_energy;}
    void setEnergy(const int& hitpoints) {m_energy = hitpoints;}
    virtual bool alive();
    virtual void damage(int severity);
private:
    int m_energy;
};

class Goodie: public Actor{
public:
    Goodie(int imageID, int startX, int startY, StudentWorld* sw);
    virtual ~Goodie() {}
    virtual void doSomething();
    int goodieTickLifetime();
    int getTicksExisted() { return m_ticksExisted; }
    virtual void pickedUp() = 0;
private:
    int m_ticksExisted;
};

class EnergyGoodie: public Goodie{
public:
    EnergyGoodie(int startX, int startY, StudentWorld* sw);
    virtual ~EnergyGoodie() {}
    virtual void pickedUp();
};

class TorpedoGoodie: public Goodie{
public:
    TorpedoGoodie(int startX, int startY, StudentWorld* sw);
    virtual ~TorpedoGoodie() {}
    virtual void pickedUp();
};

class FreeShipGoodie: public Goodie{
public:
    FreeShipGoodie(int startX, int startY, StudentWorld* sw);
    virtual ~FreeShipGoodie() {};
    virtual void pickedUp();
};

class Bullet: public Actor
{
public:
    Bullet(int startX, int startY, StudentWorld* sw,
           int firedBy, int imageID = IID_BULLET);
    virtual ~Bullet() {}
    virtual void doSomething();
    int whoFiredMe() {return m_firedBy;}
private:
    int m_firedBy;
};

class Torpedo: public Bullet
{
public:
    Torpedo(int startX, int startY, StudentWorld* sw, int firedBy);
    virtual ~Torpedo() {}
};

class PlayerShip: public Ship{
public:
    PlayerShip(StudentWorld* sw);
    virtual ~PlayerShip() {};
    virtual void doSomething();
    void gainTorpedoes(int n) { m_nTorpedoes += n; }
    void decTorpedo() {m_nTorpedoes--;}
    int getNumTorpedo() {return m_nTorpedoes;}
private:
    bool m_firedLastTime;
    int m_nTorpedoes;
};

class Alien: public Ship
{
public:
    Alien(int imageID, int startX, int startY, StudentWorld* sw);
    virtual ~Alien() {}
    bool isActive() { return m_active; }
    void setActivity(bool bl) {m_active = bl;}
private:
    bool m_active;
};

class Nachling: public Alien
{
public:
    Nachling(StudentWorld* sw, int imageID = IID_NACHLING);
    virtual ~Nachling() {}
    virtual void doSomething();
    int MDB(int x);
private:
    int m_state;
    int m_HMD;
    int m_HMR;
    int m_hdir;
};

class WealthyNachling: public Nachling
{
public:
    WealthyNachling(StudentWorld* sw);
    virtual ~WealthyNachling() {}
    virtual void doSomething();
    virtual void damage(int severity);
private:
    bool m_malfunction;
    int m_malfuncstate;
//    virtual void damage(int damagedByID) {}
};

class Smallbot: public Alien
{
public:
    Smallbot(StudentWorld* sw);
    virtual ~Smallbot() {}
    virtual void doSomething();
    virtual void damage(int severity);
private:
    bool m_gotAttacked;
};

void moveUp(Actor* who);
void moveDown(Actor* who);

#endif // _ACTOR_H_
