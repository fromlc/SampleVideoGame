//------------------------------------------------------------------------------
// Entity.cpp
//------------------------------------------------------------------------------
#include "Entity.h"

#include <string>
#include <tuple>

// prevent user Entity creation with private constructors
////////////////////////////////////////////////////////////////////////////////
Entity::Entity()
{
	// only player Entity uses constructor with no args
	m_tag = ETag::PLAYER;
	m_id = 0;
	m_scoreValue = 0;
	// initialized but not used for player
	m_isAlive = true;
	m_name = "player";
}

Entity::Entity(ETag tag, unsigned int id, const std::string& name)
	: m_tag(tag)
	, m_id(id)
	, m_name(name)
{
	m_scoreValue = 0;
	m_isAlive = true;
}

////////////////////////////////////////////////////////////////////////////////
unsigned int Entity::getId() const { return m_id; }

////////////////////////////////////////////////////////////////////////////////
ETag Entity::getTag() const { return m_tag; }

////////////////////////////////////////////////////////////////////////////////
const std::string& Entity::getName() const { return m_name; }

////////////////////////////////////////////////////////////////////////////////
void Entity::selfDestruct()
{
	m_isAlive = false;
	get<CLifespan>().remaining = 0;
}

////////////////////////////////////////////////////////////////////////////////
sf::Vector2f& Entity::pos() { return get<CTransform>().pos; }

////////////////////////////////////////////////////////////////////////////////
bool Entity::isAlive() const { return m_isAlive; }

////////////////////////////////////////////////////////////////////////////////
int Entity::getScoreValue() const { return m_scoreValue; }

////////////////////////////////////////////////////////////////////////////////
void Entity::setScoreValue(int points) { m_scoreValue = points; }

////////////////////////////////////////////////////////////////////////////////
void Entity::addPoints(int points) { m_scoreValue += points; }

// distance d from this Entity's origin to fromPos
// d^2 = distX^2 + distY^2
////////////////////////////////////////////////////////////////////////////////
float Entity::distanceTo(const sf::Vector2f& fromPos)
{
	sf::Vector2f& thisPos = pos();
	float distX = fromPos.x - thisPos.x;
	float distY = fromPos.y - thisPos.y;

	return sqrtf(distX * distX + distY * distY);
}
