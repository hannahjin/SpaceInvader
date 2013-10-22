#include "actor.h"
#include "StudentWorld.h"
#include <cstdlib>

// Students:  Add code to this file (if you wish), actor.h, StudentWorld.h, and StudentWorld.cpp

Actor::Actor(int imageID, int startX, int startY, StudentWorld* sw)
: GraphObject(imageID, startX, startY), m_studentWorld(sw), m_alive(true)
{setVisible(true);}

bool Actor::alive()
{
    if (getY() < 0 || getY() > VIEW_HEIGHT-1 || getX() < 0 || getX() > VIEW_WIDTH-1)
        kill();
    return m_alive;
}

bool operator< (const Actor& a, const Actor& b)
{
    return true;
}

// ***Star class implementations***
Star::Star(StudentWorld* sw)
: Actor(IID_STAR,rand()%30,VIEW_HEIGHT-1, sw) {}

void Star::doSomething()
{
    if (alive())
    {
        if (getY() > 0)
            moveTo(getX(),getY()-1);
        else
            kill();
    }
}
// ***Ship class implementations***
Ship::Ship(int imageID, int startX, int startY, StudentWorld* sw)
: Actor(imageID, startX, startY, sw)
{ }

bool Ship::alive()
{
    if (getEnergy() <= 0)
        kill();
    return Actor::alive();
}

void Ship::damage(int severity)
{
    if (getID() != IID_PLAYER_SHIP && severity == COLLIDE_PLAYERSHIP)
        { kill(); return;}
    int updateEnergy = getEnergy()-severity;
    setEnergy(updateEnergy);
    if (getEnergy() <= 0)
        kill();
}

// ***PlayerShip class implementations***
PlayerShip::PlayerShip(StudentWorld* sw)
: Ship(IID_PLAYER_SHIP, VIEW_WIDTH/2, 1, sw), m_firedLastTime(false), m_nTorpedoes(0)
{ setEnergy(DEFAULT_PLAYER_ENERGY); }

void PlayerShip::doSomething()
{
        // if colliding with any alien ships
    getWorld()->collide(this, getX(), getY());

        // if energy is drained
    if (getEnergy() <= 0)
    { kill(); return; }
    
        // get user's keyboard input
    int key;
    if (getWorld()->getKey(key)) // user hit a valid key
    {
        switch(key)
        {
            case KEY_PRESS_LEFT:
                if (getX() > 0)  moveTo(getX()-1, getY());
                m_firedLastTime = false;    break;
            case KEY_PRESS_RIGHT:
                if (getX() < VIEW_WIDTH-1)  moveTo(getX()+1, getY());
                m_firedLastTime = false;    break;
            case KEY_PRESS_UP:
                if (getY() < VIEW_HEIGHT-1)  moveTo(getX(), getY()+1);
                m_firedLastTime = false;    break;
            case KEY_PRESS_DOWN:
                if (getY() > 0)  moveTo(getX(), getY()-1);
                m_firedLastTime = false;    break;
            case KEY_PRESS_SPACE:
                if (!m_firedLastTime)
                {
                    if (getY() < VIEW_HEIGHT-1)
                    {
                        getWorld()->fire(IID_BULLET,IID_PLAYER_SHIP,getX(),getY()+1);
                        getWorld()->playSound(SOUND_PLAYER_FIRE);
                        m_firedLastTime = true;
                    }
                }
                break;
            case KEY_PRESS_TAB:
                if (getNumTorpedo()> 0 && !m_firedLastTime)
                {
                    getWorld()->fire(IID_TORPEDO,IID_PLAYER_SHIP,getX(),getY()+1);
                    getWorld()->playSound(SOUND_PLAYER_TORPEDO);
                    decTorpedo();
                    m_firedLastTime = true;
                }
                break;
        }
    }
    else m_firedLastTime = false; // user didn't hit a key or hit an invalid key

        // check if colliding again after shift
    getWorld()->collide(this, getX(), getY());
}

// ***Alien class implementations***
Alien::Alien(int imageID, int startX, int startY,StudentWorld* sw)
: Ship(imageID, startX, startY,sw),m_active(true) {}

Nachling::Nachling(StudentWorld* sw, int imageID)
: Alien(imageID, rand()% 30, VIEW_HEIGHT-1, sw), m_state(0)
{ setEnergy(NACHLING_STARTING_ENERGY_FACTOR * (sw->getRound())); }

WealthyNachling::WealthyNachling(StudentWorld *sw)
: Nachling(sw, IID_WEALTHY_NACHLING), m_malfunction(false)
{ setEnergy(WEALTHY_NACHLING_STARTING_ENERGY_FACTOR * (sw->getRound())); }

Smallbot::Smallbot(StudentWorld* sw)
: Alien(IID_SMALLBOT, rand()% 30, VIEW_HEIGHT-1, sw), m_gotAttacked(false)
{ setEnergy(SMALLBOT_STARTING_ENERGY_FACTOR * (sw->getRound()));}

void Nachling::doSomething()
{
    if(!isActive()) // did something last tick
    { setActivity(true); return; }
    
    setActivity(false);
    
    if (m_state == 0)
    {
        if (getX() == getWorld()->getPlayerX() && getX() != 0 && getX() != VIEW_WIDTH)
        {
            m_state = 1;
            int mdb = MDB(getX());
            if (mdb > 3)
                m_HMD = rand() % 3+1;
            else
                m_HMD = mdb;
            
            m_hdir = rand() % 2;
            m_HMR = m_HMD;
        }
        
        else if (rand() % 3 == 0)
        {
            if (getX() < getWorld()->getPlayerX())
            {  moveTo(getX()+1, getY()-1); return; }
            else if (getX() > getWorld()->getPlayerX())
            {  moveTo(getX()-1, getY()-1); return; }
        }
        
        moveTo(getX(), getY()-1);
        if (getY() < 0)
            kill();
        
        return;
    }

    
    if (m_state == 1)
    {
        if (getWorld()->getPlayerY() > getY())
        { m_state = 2; return; }
        if (m_HMR == 0)
        {
            if(m_hdir == LEFT)
                m_hdir = RIGHT;
            else    m_hdir = LEFT;
            m_HMR = 2*m_HMD;
        }
        else m_HMR--;
        if (m_hdir == LEFT)
            moveTo(getX()-1, getY());
        else moveTo(getX()+1, getY());
        int chancesOfFiring = (BASE_CHANCE_OF_FIRING / (getWorld()->getRound()) )+1;
        if (rand() % chancesOfFiring == 0)
        {
            if (getWorld()->activeEnemyProjectiles() < 2*getWorld()->getRound() )
                getWorld()->fire(IID_BULLET,IID_ALIEN,getX(),getY()-1);
        }
        if (rand() % CHANCE_TRANS_STATE2 == 0)
            m_state = 2;
        return;
    }
    
    if (m_state == 2)
    {
        if (getY() == VIEW_HEIGHT-1)
        { m_state = 0; return; }
        if (getX() == 0)
            m_hdir = RIGHT;
        else if(getX() == VIEW_WIDTH-1)
            m_hdir = LEFT;
        else
            m_hdir = rand() % 2;
        if (m_hdir == LEFT)
            moveTo(getX()-1,getY()+1);
        else moveTo(getX()+1, getY()+1);
        return;
    }
}

int Nachling::MDB(int x)
{
    return x < VIEW_WIDTH-1-x ? x : VIEW_WIDTH-1-x;
}

void WealthyNachling::doSomething()
{
    if (m_malfunction && m_malfuncstate != MALFUNC_TICKS)
    { m_malfuncstate++;  return; }
    else if (m_malfunction && m_malfuncstate == MALFUNC_TICKS)
    {    m_malfuncstate = 0; m_malfunction = false;  }
    else if (rand() % 200 == 0)
    {
        m_malfunction = true;
        m_malfuncstate = 0;
        return;
    }
    Nachling::doSomething();
}

void Smallbot::doSomething()
{
    if (!isActive())
    { setActivity(true); return; }
    setActivity(false);
    
    if(m_gotAttacked)
    {
        if (getX() == 0)
            moveTo(1, getY()-1);
        else if(getX() == VIEW_WIDTH-1 || rand() % 2 == 0)
            moveTo(getX()-1, getY()-1);
        else 
            moveTo(getX()+1, getY()-1);
    }
    else
        moveTo(getX(), getY()-1);
    m_gotAttacked = false;
    
    if (getX() == getWorld()->getPlayerX() && getY() > getWorld()->getPlayerY())
    {
        int q = 100 / (getWorld()->getRound());
        int missileType = IID_BULLET;
        if (rand() % q == 0)
            missileType = IID_TORPEDO;
        int d = getWorld()->activeEnemyProjectiles();
        if (d < 2 * getWorld()->getRound())
            getWorld()->fire(missileType, IID_SMALLBOT, getX(), getY()-1);
        
    }
    alive();
}

void WealthyNachling::damage(int severity)
{
    if (severity != COLLIDE_PLAYERSHIP)
    {
        Alien::damage(severity);
        if (!alive() && rand() % 3 == 0)
        {
            if (rand() % 100 < 50)
                getWorld()->dropGoodie(IID_ENERGY_GOODIE,getX(),getY());
            else
                getWorld()->dropGoodie(IID_TORPEDO_GOODIE,getX(),getY());
        }
    }
    else kill(); // collide with player ship
    if (alive())
        getWorld()->playSound(SOUND_ENEMY_HIT);
}

void Smallbot::damage(int severity)
{
    if (severity != COLLIDE_PLAYERSHIP)
    {
        m_gotAttacked = true;
        Alien::damage(severity);
        if (!alive() && rand() % 3 == 0)
            getWorld()->dropGoodie(IID_FREE_SHIP_GOODIE,getX(),getY());
    }
    else Alien::damage(severity); // collide with player ship
    if (alive())
        getWorld()->playSound(SOUND_ENEMY_HIT);
}

 // Implementations of helper functions of Actor class
void moveUp(Actor* who)
{
    if (who->getY() < VIEW_HEIGHT-1)
        who->moveTo(who->getX(),who->getY()+1);
    else // moving off the grid
        who->kill();
}

void moveDown(Actor* who)
{
    if (who->getY() > 0)
        who->moveTo(who->getX(),who->getY()-1);
    else // moving off the grid
        who->kill();
}

// ***Bullet class implementations***
Bullet::Bullet(int startX, int startY,StudentWorld* sw,
               int firedBy, int imageID)
: Actor(imageID, startX, startY, sw), m_firedBy(firedBy) {alive();}

void Bullet::doSomething()
{
    if (!getWorld()->collide(this, getX(), getY())) // didn't collide with a target
    {
        if (m_firedBy == IID_PLAYER_SHIP)
            moveUp(this);
        else
            moveDown(this);
        getWorld()->collide(this, getX(), getY());
    }
}

// ***Torpedo class implementations***
Torpedo::Torpedo(int startX, int startY, StudentWorld* sw, int firedBy)
: Bullet(startX, startY, sw, firedBy, IID_TORPEDO) {}

// *** Goodie class implementations ***
Goodie::Goodie(int imageID, int startX, int startY, StudentWorld* sw)
: Actor(imageID,startX,startY,sw), m_ticksExisted(1) {}

int Goodie:: goodieTickLifetime()
{ return 100 / (getWorld()->getRound()) + 30; }

void Goodie::doSomething() {
    if (!getWorld()->collide(this, getX(), getY()) )
    {
        m_ticksExisted++;
        double brightness = static_cast<double>(goodieTickLifetime()-getTicksExisted()) / goodieTickLifetime() + 0.2;
        setBrightness(brightness);
        if (getTicksExisted() == goodieTickLifetime())
        {  kill(); return; }
        else if (getTicksExisted() % 3 == 0)
            moveTo(getX(), getY()-1);
        if (alive())
            getWorld()->collide(this, getX(), getY());
    }
}

FreeShipGoodie::FreeShipGoodie(int startX, int startY, StudentWorld* sw)
: Goodie(IID_FREE_SHIP_GOODIE, startX, startY, sw) {}

TorpedoGoodie::TorpedoGoodie(int startX, int startY, StudentWorld* sw)
: Goodie(IID_TORPEDO_GOODIE, startX, startY, sw) {}

EnergyGoodie::EnergyGoodie(int startX, int startY, StudentWorld* sw)
: Goodie(IID_ENERGY_GOODIE, startX, startY, sw) {}

//void FreeShipGoodie::doSomething()
//{
//    if (!getWorld()->collide(this, getX(), getY()) )
//         Goodie::doSomething();
//}

void FreeShipGoodie::pickedUp()
{ getWorld()->incLives(); }

void EnergyGoodie::pickedUp()
{ getWorld()->refillPlayerEnergy(); }

void TorpedoGoodie::pickedUp()
{ getWorld()->givePlayerTorpedoes(); }




