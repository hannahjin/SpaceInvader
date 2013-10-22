#ifndef _STUDENTWORLD_H_
#define _STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <set>

class Actor;
class Alien;
class PlayerShip;

const unsigned int KILL_NACHLING = 1000;
const unsigned int KILL_WEALTHY_NACHLING = 1200;
const unsigned int KILL_SMALLBOT = 1500;

// Students:  Add code to this file, StudentWorld.cpp, actor.h, and actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld();
    virtual ~StudentWorld();
	virtual void init();
	virtual int move();
	virtual void cleanUp();
    
    int getRound() const {return m_round;}
    void setDisplayText();
    int getPlayerX() const;
    int getPlayerY() const;
    int getnAliens() const {return m_nAliens;}
    int maxAliens() const;
    int roundAdvanceBar() const;
    void advanceRound();
    int activeEnemyProjectiles() const {return m_nEnemyProjectiles;}
    void refillPlayerEnergy();
    void givePlayerTorpedoes();

    void addAliensOrStars();
    void dropGoodie(int imageID, int x, int y);
    bool collide(Actor* caller, int x, int y);
    void fire(int missileType, int firedBy,int x, int y);

private:
    std::set<Actor*> m_actors;
    PlayerShip* m_player;
    int m_round;
    int m_nAliens;
    int m_nAliensDestroyedThisRound;
    int m_nEnemyProjectiles;
};

unsigned int addScore(int imageID);

#endif // _STUDENTWORLD_H_
