#include "StudentWorld.h"
#include "actor.h"
#include <iomanip>
#include <sstream>
#include <iostream>
using namespace std;

GameWorld* createStudentWorld()
{
    return new StudentWorld();
}

StudentWorld::StudentWorld()
: m_round(1), m_nAliens(0), m_nAliensDestroyedThisRound(0), m_nEnemyProjectiles(0)
{}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

void StudentWorld::init()
{
    m_player = new PlayerShip(this);
    m_nAliens = 0;
    m_nAliensDestroyedThisRound = 0;
    m_nEnemyProjectiles = 0;
}

int StudentWorld::move()
{
    // add new actors (aliens or stars)
    addAliensOrStars();
    
    // updateDisplayText();  // update the score/lives/round at screen top
    setDisplayText();
    
    // current actors doSomething
    if (!m_player->alive())
        return GWSTATUS_PLAYER_DIED;
    else m_player->doSomething();
    
    set<Actor*>::iterator it = m_actors.begin();
    while(it != m_actors.end())
    {
        if ((*it)->alive())
            (*it)->doSomething();
        it++;
    }
    
        //delete dead actors
    it = m_actors.begin();
    while(it != m_actors.end())
    {
        if (! (*it)->alive())
        {
            Alien* ap = dynamic_cast<Alien*>(*it);
            if (ap!=NULL) // an alien died
                m_nAliens--;

            delete *it;
            it = m_actors.erase(it);
        }
        else it++;
    }
    
    if ( m_nAliensDestroyedThisRound == 4*getRound() )
        advanceRound();
    
        // determine if player has died
    return m_player->alive() ? GWSTATUS_CONTINUE_GAME : GWSTATUS_PLAYER_DIED;
}

void StudentWorld::cleanUp()
{
    set<Actor*>::iterator it = m_actors.begin();
    while (it != m_actors.end())
    {
        delete *it;
        it = m_actors.erase(it);
    }
    delete m_player;
    decLives();
}

int StudentWorld::getPlayerX() const {return m_player->getX();}
int StudentWorld::getPlayerY() const {return m_player->getY();}

void StudentWorld::setDisplayText()
{
    ostringstream ss;
    ss << "Score: " << setw(7) << setfill('0') << getScore() << "  "
       << "Round: " << setw(2) << setfill('0') << getRound() << "  "
       << "Energy: " << (static_cast<double>(m_player->getEnergy())/DEFAULT_PLAYER_ENERGY)*100
       << "%" << "  "
       << "Torpedoes: " << setw(3) << setfill('0') << m_player->getNumTorpedo() << "  "
       << "Ships: " << setw(2) << setfill('0') << getLives();
    string s = ss.str();
    
    // string s = displayTextFormatting(score, round, energyPercent, torpedoes, shipsLeft);
    // Finally, update the display text at the top of the screen with your // newly created stats
    setGameStatText(s); // calls GameWorld::setGameStatText
}

int StudentWorld::maxAliens() const  { return 2 + (.5*getRound()); }
int StudentWorld::roundAdvanceBar() const { return 4 * getRound(); }

void StudentWorld::advanceRound()
{
    m_round++;
    m_nAliensDestroyedThisRound = 0;    
}

void StudentWorld::addAliensOrStars()
{
    if (testParamsProvided())
    {
        if (!m_actors.empty()) // one non-Player object already exists
            return;
        // add a test parameter required actor
        int parameter = getTestParam(TEST_PARAM_ACTOR_INDEX);
        switch (parameter) {
            case TEST_PARAM_NACHLING:
                m_actors.insert(new Nachling(this));        m_nAliens++;                break;
            case TEST_PARAM_WEALTHY_NACHLING:
                m_actors.insert(new WealthyNachling(this)); m_nAliens++;                break;
            case TEST_PARAM_SMALLBOT:
                m_actors.insert(new Smallbot(this));        m_nAliens++;                break;
            case TEST_PARAM_GOODIE_ENERGY:
                m_actors.insert(new EnergyGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1, this));   break;
            case TEST_PARAM_GOODIE_TORPEDO:
                m_actors.insert(new TorpedoGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1,this));   break;
            case TEST_PARAM_GOODIE_FREE_SHIP:
                m_actors.insert(new FreeShipGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1,this));  break;
            default:
                return;
        }
    }
    else  // no test parameter provided
    {
        int nAliensNeedToDestroy = roundAdvanceBar()-m_nAliensDestroyedThisRound;
        if (getnAliens() < maxAliens() && getnAliens() < nAliensNeedToDestroy) // add aliens to the field
        {
            int p1 = rand() % 100;
            if (p1 < 70)
            {
                int p2 = rand() % 100;
                if (p2 < 20)
                    m_actors.insert(new WealthyNachling(this));
                else
                    m_actors.insert(new Nachling(this));
            }
            else
                m_actors.insert(new Smallbot(this));
            m_nAliens++;
        }
        if (rand() % 100 < 33) // add a star
            m_actors.insert(new Star(this));
    }
}

void StudentWorld::fire(int missileType, int firedBy, int x, int y)
{
    if (y < 0 || y > VIEW_HEIGHT-1)
        return;
    if (firedBy == IID_ALIEN)
    {
        m_nEnemyProjectiles++; // increase number of active enemy projectiles
        playSound(SOUND_ENEMY_FIRE); // check spec again
    }
    else playSound(SOUND_PLAYER_FIRE); // check spec again
    
    if (missileType == IID_BULLET)
        m_actors.insert(new Bullet(x, y, this, firedBy));
    if (missileType == IID_TORPEDO)
        m_actors.insert(new Bullet(x, y, this, firedBy, IID_TORPEDO));
}

bool StudentWorld::collide(Actor* caller, int x, int y)
{
    bool didCollide = false;
    if (caller == m_player)
    {
        if (m_actors.empty() || !m_player->alive())
            return false;
        set<Actor*>::iterator it = m_actors.begin();
        while (it != m_actors.end())
        {
            Alien* ap = dynamic_cast<Alien*>(*it);
            if (ap != NULL && ap->alive() &&
                m_player->getX() == (*it)->getX() && m_player->getY() == (*it)->getY())
            {
                didCollide = true;
                playSound(SOUND_ENEMY_PLAYER_COLLISION);
                m_player->damage(COLLIDE_ALIEN_DAMAGE);
                ap->damage(COLLIDE_PLAYERSHIP);
            }
            if (m_player->alive())
                it++;
            else
                return didCollide;
        }
        return didCollide;
    }

    Bullet *bulletp = dynamic_cast<Bullet*>(caller);
    if (bulletp != NULL)
    {
        if (!bulletp->alive())
        { m_nEnemyProjectiles--; return false; }
        Torpedo* tp = dynamic_cast<Torpedo*>(bulletp);
        int damageLevel = COLLIDE_BULLET_DAMAGE;
        if (tp != NULL) // this is a torpedo
            damageLevel = COLLIDE_TORPEDO_DAMAGE;
        
        if (bulletp->whoFiredMe() == IID_PLAYER_SHIP) // player's bullet
        {
            set<Actor*>::iterator it = m_actors.begin();
            while (it != m_actors.end())
            {
                Alien* alienp = dynamic_cast<Alien*>(*it);
                if (alienp != NULL && alienp->alive() && bulletp->getX() == (*it)->getX()
                    && bulletp->getY() == (*it)->getY())
                {
                    didCollide = true;
                    alienp->damage(damageLevel);
                    if (!alienp->alive()){
                        playSound(SOUND_ENEMY_DIE);
                        increaseScore(addScore(alienp->getID()));
                        m_nAliensDestroyedThisRound++;
                    }
                }
                it++;
            }
        }
        else if (bulletp->whoFiredMe() == IID_ALIEN) // alien's bullet
        {
            if (bulletp->getX() == getPlayerX() && bulletp->getY() == getPlayerY()){
                m_player->damage(damageLevel);
                m_nEnemyProjectiles--;
                didCollide = true;
            }
        }
            
        if (didCollide) { bulletp->kill(); }
        return didCollide;
    }
    
    Goodie *goodiep = dynamic_cast<Goodie*>(caller);
    if (goodiep != NULL)
    {
        if (!goodiep->alive())
            return false;
        if (goodiep->getX() == m_player->getX() && goodiep->getY() == m_player->getY())
        {
            didCollide = true;
            playSound(SOUND_GOT_GOODIE);
            increaseScore(GOODIE_SCORE);
            goodiep->pickedUp();
            // something happens
            goodiep->kill();
        }
        return didCollide;
    }
    return didCollide;
}

void StudentWorld::dropGoodie(int imageID, int x, int y)
{
    if (imageID == IID_ENERGY_GOODIE)
        m_actors.insert(new EnergyGoodie(x, y, this));
    else if (imageID == IID_FREE_SHIP_GOODIE)
        m_actors.insert(new FreeShipGoodie(x, y, this));
    else if (imageID == IID_TORPEDO_GOODIE)
        m_actors.insert(new TorpedoGoodie(x, y, this));
        
}

void StudentWorld::refillPlayerEnergy()
{  m_player->setEnergy(DEFAULT_PLAYER_ENERGY); }

void StudentWorld::givePlayerTorpedoes()
{  m_player->gainTorpedoes(TORPEDO_GOODIE_PACKAGE_NUM); }

unsigned int addScore(int imageID)
{
    if (imageID == IID_NACHLING)
        return KILL_NACHLING;
    else if (imageID == IID_WEALTHY_NACHLING)
        return KILL_WEALTHY_NACHLING;
    else // SMALLBOT
        return KILL_SMALLBOT;
}



