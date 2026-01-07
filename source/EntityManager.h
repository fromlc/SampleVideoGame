//------------------------------------------------------------------------------
// EntityManager.h
//------------------------------------------------------------------------------
#pragma once

#include "Entity.h"
#include "ConfigsManager.h"

#include <SFML/Graphics.hpp>

#include <map>
#include <memory>
#include <vector>

// each Entity tag maps to a vector of Entities
typedef std::vector<spEntity> EntityVector;
// unordered_map has reserve() member function, map does not
typedef std::unordered_map<ETag, EntityVector> EntityMap;

// reserve vector memory to prevent reallocation
constexpr unsigned int HOLDING_VECTOR_RESERVE = 1024;
constexpr unsigned int ENTITY_MAP_RESERVE = 4096;

// random Entity colors available
constexpr unsigned int ENTITY_RANDOM_COLOR_COUNT = 8;

////////////////////////////////////////////////////////////////////////////////
class EntityManager
{
private:
	// wait one frame to add to m_entityMap to keep iterator valid
	EntityVector m_entitiesToAdd;
	EntityMap	 m_entityMap;
	unsigned int m_totalEntities;
	Configs&	 m_cfg;

public:
	EntityManager(Configs& cfg);

	// random Entity colors selected from these
	sf::Color m_colors[ENTITY_RANDOM_COLOR_COUNT];

	spEntity addEntity(ETag tag, const std::string& name);
	void removeDeadEntities(ETag tag);
	void removeAllEntities();

	EntityVector& getEntities(ETag tag);
	EntityMap& getEntityMap();
	void updateEntityMap();

	inline sf::Vector2f getRandomXYSpeeds();
	void spawnEnemy(sf::Vector2u frameSize);
	void spawnSmallEnemies(spEntity e);
	void spawnBullet(sf::Vector2f playerPos);
	void spawnSpecialWeapon();
};
