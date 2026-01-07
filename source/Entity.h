//------------------------------------------------------------------------------
// Entity.h
//------------------------------------------------------------------------------
#pragma once

#include "Components.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <tuple>
#include <vector>

// kinds of Entity plus last one for iterator
////////////////////////////////////////////////////////////////////////////////
enum class ETag
{
	PLAYER, ENEMY, SMALLENEMY, BULLET, XYZZY
};

////////////////////////////////////////////////////////////////////////////////
typedef std::shared_ptr<class Entity> spEntity;

////////////////////////////////////////////////////////////////////////////////
class Entity
{
private:
	unsigned int	m_id;
	ETag			m_tag;
	int				m_scoreValue;	// can be negative
	bool			m_isAlive;
	std::string		m_name;

	// prevent user Entity creation with private constructor
	friend class EntityManager;

	Entity(ETag tag, unsigned int id, const std::string& name);

public:
	Entity();

	ComponentTuple components = std::make_tuple(
		CTransform{}		// pos, velocity, angle
		, CShape{}			// sf::CircleShape circle
		, CCollision{}		// collides with others
		, CMeta{}			// smallEnemy parent info
		, CLifespan{}		// total and remaining frames
		, CInput{}			// only player has CInput
		, CSpecialWeapon{}	// only player has CSpecialWeapon
	);

	unsigned int getId() const;
	ETag getTag() const;
	const std::string& getName() const;

	sf::Vector2f& pos();

	bool isAlive() const;
	void selfDestruct();

	int getScoreValue() const;
	void setScoreValue(int points);
	void addPoints(int points);

	float distanceTo(const sf::Vector2f& fromPos);

	// returns shared_ptr to Component data
	template <typename T>
	T& get();

	// activates Component and sets its properties
	// copies stack data into Component structure
	template <typename T>
	void add(T);

	// returns true when specified Component is active
	template <typename T>
	bool has() const;
};

// template function definitions

// returns Component reference
////////////////////////////////////////////////////////////////////////////////
template <typename T>
T& Entity::get() 
{
	return std::get<T>(components); 
}

// activates Component and sets its properties
////////////////////////////////////////////////////////////////////////////////
template <typename T>
void Entity::add(T data)
{
	std::get<T>(components) = data;

	// mark as valid Component for this Entity 
	std::get<T>(components).exists = true;
}

// returns true when specified Component is active
////////////////////////////////////////////////////////////////////////////////
template <typename T>
bool Entity::has() const 
{ 
	return std::get<T>(components).exists; 
}
