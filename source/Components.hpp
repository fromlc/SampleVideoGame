//------------------------------------------------------------------------------
// Components.hpp
//------------------------------------------------------------------------------
#pragma once

#include <SFML/Graphics.hpp>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
class Component
{
public:
	bool exists;
	Component() { exists = false; }
};

////////////////////////////////////////////////////////////////////////////////
class CTransform : public Component
{
public:
	sf::Vector2f pos{};
	sf::Vector2f velocity{};
	//sf::Vector2f scale = {};
	float angle = 0;

	CTransform() = default;
	CTransform(const sf::Vector2f& p, const sf::Vector2f& v, float a)
		: pos(p)
		, velocity(v)
		, angle(a)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CTransform& rhs)
	{
		pos = rhs.pos;
		velocity = rhs.velocity;
		angle = rhs.angle;
		exists = rhs.exists;
	}
};

////////////////////////////////////////////////////////////////////////////////
class CShape : public Component
{
public:
	sf::CircleShape circle{};

	CShape() = default;
	CShape(float radius, size_t vertices, sf::Color fillColor,
		   sf::Color outlineColor, float outlinePx)
	{
		circle.setRadius(radius);
		circle.setPointCount(vertices);
		circle.setFillColor(fillColor);
		circle.setOutlineColor(outlineColor);
		circle.setOutlineThickness(outlinePx);
		circle.setOrigin({ radius, radius });

		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CShape& rhs)
	{ 
		circle = rhs.circle;
		exists = rhs.exists;
	}
};

////////////////////////////////////////////////////////////////////////////////
class CMeta : public Component
{
public:
	unsigned int spawnerId;

	CMeta() = default;
	CMeta(unsigned int sid) : spawnerId(sid)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	 void operator=(CMeta& rhs) 
	 { 
		 spawnerId = rhs.spawnerId;
		 exists = rhs.exists;
	 }
};

////////////////////////////////////////////////////////////////////////////////
class CCollision : public Component
{
public:
	float radius = 0;

	CCollision() = default;
	CCollision(float r)	: radius(r)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CCollision& rhs) 
	{
		radius = rhs.radius; 
		exists = rhs.exists;
	}
};

////////////////////////////////////////////////////////////////////////////////
class CInput : public Component
{
public:
	bool left;
	bool right;
	bool shoot;

	CInput() = default;
	CInput(bool l, bool r, bool s)
		: left(l)
		, right(r)
		, shoot(s)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CInput& rhs)
	{
		left  = rhs.left;
		right = rhs.right;
		shoot = rhs.shoot;
		exists = rhs.exists;
	}
};

////////////////////////////////////////////////////////////////////////////////
class CLifespan : public Component
{
public:
	unsigned int total = 0;
	unsigned int remaining = 0;
	unsigned int spawnerId = 0;

	CLifespan() = default;
	CLifespan(unsigned int totalLifespan)
		: total(totalLifespan)
		, remaining(totalLifespan)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CLifespan& rhs)
	{
		total = rhs.total;
		remaining = rhs.remaining;
		exists = rhs.exists;
	}
};

// special behavior
////////////////////////////////////////////////////////////////////////////////
class CSpecialWeapon : public Component
{
public:
	int xyzzy = 0;

	CSpecialWeapon() = default;
	CSpecialWeapon(int spec)
		: xyzzy(spec)
	{
		exists = true;
	}
	// assignment operator for Entity::add<T>()
	void operator=(CSpecialWeapon& rhs)
	{
		xyzzy = rhs.xyzzy;
		exists = rhs.exists;
	}
};

// Component tuple stored in each Entity
//	- different Entity tags can contain different data
//	- system functions can process all Entities in one loop
//			that checks valid Component data is present
////////////////////////////////////////////////////////////////////////////////
using ComponentTuple = std::tuple<
	CTransform			// pos, velocity, angle
	, CShape			// sf::CircleShape circle
	, CCollision		// collides with others
	, CMeta				// smallEnemy parent info
	, CLifespan			// total and remaining frames
	, CInput			// only player has CInput
	, CSpecialWeapon	// only player has CSpecialWeapon
>;
