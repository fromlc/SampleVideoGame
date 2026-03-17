//------------------------------------------------------------------------------
// EntityManager.cpp
//------------------------------------------------------------------------------
#include "EntityManager.h"

#include <algorithm>
#include <vector>

// constructor
////////////////////////////////////////////////////////////////////////////////
EntityManager::EntityManager(Configs &cfg) : m_cfg(cfg)
{
    m_totalEntities = 0;

    //#TODO need better way to initialize array outside header file
    m_colors[0] = sf::Color(240, 245, 60);	// soft yellow
    m_colors[1] = sf::Color(105, 190, 255);	// medium blue
    m_colors[2] = sf::Color(45, 60, 200);	// dark blue
    m_colors[3] = sf::Color(0, 200, 120);	// medium green
    m_colors[4] = sf::Color(0, 200, 120);	// medium green
    m_colors[5] = sf::Color(240, 90, 170);	// pink
    m_colors[6] = sf::Color(125, 10, 180);	// purple
    m_colors[7] = sf::Color::Transparent;

    m_entitiesToAdd.reserve(HOLDING_VECTOR_RESERVE);

    // reserve Entity vector memory
    //#TODO need way to reserve with iteration over Etag enum class
    m_entityMap[ETag::ENEMY].reserve(ENTITY_MAP_RESERVE);
    m_entityMap[ETag::SMALLENEMY].reserve(ENTITY_MAP_RESERVE);
    m_entityMap[ETag::BULLET].reserve(ENTITY_MAP_RESERVE);
    //#TODO implement special weapon first
    //m_entityMap[ETag::XYZZY].reserve(ENTITY_MAP_RESERVE);
}

// creates new Entity and puts it in holding vector
////////////////////////////////////////////////////////////////////////////////
spEntity EntityManager::addEntity(ETag tag, const std::string &name)
{
    // since private Entity constructor, EntityManager
    // must create Entity instance with new keyword
    spEntity e(new Entity(tag, ++m_totalEntities, name));

    m_entitiesToAdd.push_back(e);

    return e;
}

////////////////////////////////////////////////////////////////////////////////
void EntityManager::removeDeadEntities(ETag tag)
{
    EntityVector &ev = getEntities(tag);

    EntityVector::iterator newEnd =
        std::remove_if(ev.begin(), ev.end(),
                       [](spEntity e) { return !(e->isAlive()); });

    ev.erase(newEnd, ev.end());
}

////////////////////////////////////////////////////////////////////////////////
void EntityManager::removeAllEntities()
{
    m_entityMap.clear();
    m_entitiesToAdd.clear();
}

////////////////////////////////////////////////////////////////////////////////
EntityMap &EntityManager::getEntityMap()
{
    return m_entityMap;
}

// returns vector of entities with specified tag
// if tag is not a map key, 
//		adds new key, value pair, new value is empty vector
//		returns reference to empty vector
////////////////////////////////////////////////////////////////////////////////
EntityVector &EntityManager::getEntities(ETag tag)
{
    return m_entityMap[tag];
}

// - called at beginning of each frame
// - remove Entities with no lifespan remaining
// - add Entities in holding vector
// - Entities added are available to use this frame
////////////////////////////////////////////////////////////////////////////////
void EntityManager::updateEntityMap()
{
    // add waiting Entities to map and clear holding vector
    for (spEntity e : m_entitiesToAdd)
        m_entityMap[e->getTag()].push_back(e);

    m_entitiesToAdd.clear();
}

// returns random speed between configs min and max
////////////////////////////////////////////////////////////////////////////////
inline sf::Vector2f EntityManager::getRandomXYSpeeds()
{
    int speedRange = (int)(m_cfg.enemy.maxSpeed - m_cfg.enemy.minSpeed);

    // if maxSpeed <= minSpeed due to ImGui controls use rand() % 4
    if (speedRange <= 0)
        speedRange = 4;

    sf::Vector2f randomSpeed(
        rand() % speedRange + m_cfg.enemy.minSpeed,
        rand() % speedRange + m_cfg.enemy.minSpeed);

    return randomSpeed;
}

// spawn one enemy Entity and set values from configs
////////////////////////////////////////////////////////////////////////////////
void EntityManager::spawnEnemy(sf::Vector2u frameSize)
{
    sf::Vector2f randomSpeed;
    sf::Vector2f randomPos;
    unsigned int randomAngle;
    unsigned int colorIndex;
    int spawnCount = m_cfg.enemy.spawnCount;

    for (int i = 0; i < spawnCount; i++)
    {
        spEntity e = addEntity(ETag::ENEMY, "enemy");

        // random speed between configs min and max
        randomSpeed = getRandomXYSpeeds();

        // random starting screen position
        sf::Vector2f randomPos(
            frameSize.x - (m_cfg.enemy.collisionRadius * 2)
            , frameSize.y - (m_cfg.enemy.collisionRadius * 2));

        randomPos.x = rand() % (int)randomPos.x + m_cfg.enemy.collisionRadius;
        randomPos.y = rand() % (int)randomPos.y + m_cfg.enemy.collisionRadius;

        randomAngle = rand() % 360;

        e->add<CTransform>({ randomPos, randomSpeed, (float)randomAngle });

        // set up enemy CShape Component

        // make random fill color from presets in m_colors
        colorIndex = rand() % (ENTITY_RANDOM_COLOR_COUNT - 1);

        // random number of vertices
        size_t vertices = rand()
            % (m_cfg.enemy.maxVertices - m_cfg.enemy.minVertices + 1)
            + m_cfg.enemy.minVertices;

        e->add<CShape>(
            { m_cfg.enemy.shapeRadius
            , vertices
            , m_colors[colorIndex]
            , m_cfg.enemy.outlineColor
            , m_cfg.enemy.outlinePx
            });

        e->add<CCollision>(
            { m_cfg.enemy.shapeRadius + m_cfg.enemy.outlinePx });

        // lifespan is at min half configs lifespan, max follows configs
        e->add<CLifespan>({ (unsigned int)m_cfg.enemy.smallLifespan * 2 });

        // enemy point value is configs * number of vertices
        e->setScoreValue((int)(m_cfg.enemy.scoreValue * (int)vertices));
    }
}

// spawns smallEnemy starburst when Entity e collides
////////////////////////////////////////////////////////////////////////////////
void EntityManager::spawnSmallEnemies(spEntity e)
{
    // spawn smallEnemy starburst at spawning enemy's position
    CTransform &rTrans = e->get<CTransform>();
    sf::CircleShape &circle = e->get<CShape>().circle;

    // smallEnemy is half the size of spawning enemy
    float seRadius = circle.getRadius() / 2;
    size_t seVertices = circle.getPointCount();
    // use spawning enemy color with full opacity
    sf::Color eFillColor = circle.getFillColor();
    eFillColor.a = 255;
    sf::Color eOutColor = circle.getOutlineColor();
    float seOutlinePx = circle.getOutlineThickness();

    // radians = angleDegrees * (PI / 180 degrees)
    constexpr float radiansConversion = 3.14159265f / 180.0f;
    float addDegrees = 360.0f / seVertices;
    // start with spawning enemy's angle
    float nextAngle = e->get<CTransform>().angle;

    // prep boilerplate Components for all these smallEnemies
    CShape cshape({ seRadius, seVertices
        , eFillColor, eOutColor, seOutlinePx });
    CCollision collision({ seRadius + seOutlinePx });
    CLifespan lifespan({ (unsigned int)m_cfg.enemy.smallLifespan });
    CMeta meta(e->getId());

    // smallEnemy has twice point value of spawning enemy
    int scoreValue = e->getScoreValue() * 2;

    // smallEnemy starburst
    for (int i = 0; i < seVertices; i++, nextAngle += addDegrees)
    {
        spEntity se = addEntity(ETag::SMALLENEMY, "small");
        float radianAngle = nextAngle * radiansConversion;

        // set initial angle, random x and y velocities
        sf::Vector2f velocity(cos(radianAngle), sin(radianAngle));
        sf::Vector2f randomSpeed(getRandomXYSpeeds());
        velocity.x *= randomSpeed.x;
        velocity.y *= randomSpeed.y;

        se->add<CTransform>({ rTrans.pos, velocity, nextAngle });

        se->add<CShape>(cshape);
        se->add<CCollision>(collision);
        se->add<CLifespan>(lifespan);
        se->add<CMeta>(meta);
        se->setScoreValue(scoreValue);
    }
}

// spawns a bullet from player Entity to a target location
//	- target is mouse click position
//	- bullet speed is given as a scalar speed
//	- bullet x and y velocity set from player-to-target angle
////////////////////////////////////////////////////////////////////////////////
void EntityManager::spawnBullet(sf::Vector2f playerPos)
{
    spEntity b = addEntity(ETag::BULLET, "bullet");

    // bullet originates at player position
    b->add<CTransform>(
        { playerPos, { 0, -m_cfg.bullet.speed }, 90 });

    b->add<CShape>(
        { m_cfg.bullet.shapeRadius
        , m_cfg.bullet.vertices
        , m_cfg.bullet.fillColor
        , m_cfg.bullet.outlineColor
        , m_cfg.bullet.outlinePx
        });

    b->add<CCollision>(
        { m_cfg.bullet.shapeRadius + m_cfg.bullet.outlinePx });

    b->add<CLifespan>({ m_cfg.bullet.lifespan });
}

//#TODO implement special weapon
////////////////////////////////////////////////////////////////////////////////
void EntityManager::spawnSpecialWeapon()
{ }
