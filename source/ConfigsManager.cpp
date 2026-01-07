//------------------------------------------------------------------------------
// ConfigsManager.cpp
//------------------------------------------------------------------------------
#include "ConfigsManager.h"
#include "errors.h"

#include <fstream>
#include <iostream>
#include <string>

// globals
////////////////////////////////////////////////////////////////////////////////
// values are 0-255 but reading uint8_t from file makes display problems
static unsigned int g_R, g_G, g_B;

// constructor loads configs from file
////////////////////////////////////////////////////////////////////////////////
ConfigsManager::ConfigsManager(const std::string& cfgFile)
{
	std::cout << "Loading config data from file " << cfgFile << "\n";

	std::ifstream fin(cfgFile);
	if (!fin)
	{
		std::cerr << "Could not open configuration file " 
			<< cfgFile << "\n";
		exit(ERRCODE::FILE_NOT_OPEN);
	}

	// first line token is config type
	char cfgType;
	while (fin >> cfgType)
	{
		switch (cfgType)
		{
		case 'W': loadWindowConfigs(fin);	break;
		case 'F': loadFontConfigs(fin);		break;
		case 'P': loadPlayerConfigs(fin);	break;
		case 'E': loadEnemyConfigs(fin);	break;
		case 'B': loadBulletConfigs(fin);	break;
		case '#': eatCommentLine(fin);		break;
		default:
			std::cerr << "Ignoring unknown config type: " << cfgType << "\n";
		}
	}

	fin.close();
}

////////////////////////////////////////////////////////////////////////////////
Configs& ConfigsManager::getConfigs()
{
	return m_configs;
}

////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::loadWindowConfigs(std::ifstream& fin)
{
	fin >> m_configs.window.frameWidth >> m_configs.window.frameHeight
		>> m_configs.window.frameLimit;

	// add room for ImGui windows
	m_configs.window.width = m_configs.window.frameWidth + containingSizeX;
	m_configs.window.height = m_configs.window.frameHeight;

	m_configs.window.print();
}

////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::loadFontConfigs(std::ifstream& fin)
{
	// read entire expected line from file
	fin >> m_configs.font.fontFile >> m_configs.font.fontSize
		>> g_R >> g_G >> g_B;

	m_configs.font.fillColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	m_configs.font.print();
}

////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::loadPlayerConfigs(std::ifstream& fin)
{
	// read entire expected line from file
	fin >> m_configs.player.shapeRadius >> m_configs.player.lives
		>> m_configs.player.speed >> g_R >> g_G >> g_B;

	m_configs.player.fillColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	fin	>> g_R >> g_G >> g_B
		>> m_configs.player.outlinePx >> m_configs.player.vertices;

	m_configs.player.outlineColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	m_configs.player.collisionRadius =
		m_configs.player.shapeRadius + m_configs.player.outlinePx;
	
	m_configs.player.print();
}

////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::loadEnemyConfigs(std::ifstream& fin)
{
	// read entire expected line from file
	fin >> m_configs.enemy.shapeRadius >> m_configs.enemy.scoreValue
		>> m_configs.enemy.minSpeed >> m_configs.enemy.maxSpeed
		>> g_R >> g_G >> g_B >> m_configs.enemy.outlinePx
		>> m_configs.enemy.minVertices >> m_configs.enemy.maxVertices
		>> m_configs.enemy.smallLifespan 
		>> m_configs.enemy.spawnInterval >> m_configs.enemy.spawnCount;

	m_configs.enemy.outlineColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	m_configs.enemy.collisionRadius =
		m_configs.enemy.shapeRadius + m_configs.enemy.outlinePx;

	m_configs.enemy.print();
}

////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::loadBulletConfigs(std::ifstream& fin)
{
	// read entire expected line from file
	fin >> m_configs.bullet.shapeRadius >>
		m_configs.bullet.lifespan >> m_configs.bullet.speed
		>> g_R >> g_G >> g_B;

	m_configs.bullet.fillColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	fin	>> g_R >> g_G >> g_B >> 
		m_configs.bullet.outlinePx >> m_configs.bullet.vertices;

	m_configs.bullet.outlineColor = { (uint8_t)g_R, (uint8_t)g_G, (uint8_t)g_B };

	m_configs.bullet.collisionRadius =
		m_configs.bullet.shapeRadius + m_configs.bullet.outlinePx;

	m_configs.bullet.print();
}

//#TODO comment line cannot have spaces
////////////////////////////////////////////////////////////////////////////////
void ConfigsManager::eatCommentLine(std::ifstream& fin)
{
	std::string comment;
	fin >> comment;

	std::cout << "\nComment\n"
		<< " " << comment << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void WindowConfigs::print() const
{
	std::cout << "\nWindow\n"
		<< " Width: " << frameWidth << " Height: " << frameHeight << "\n"
		<< " Frame Limit: " << frameLimit << "/sec" << "\n"
		<< " Full Screen: " << fullScreen << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void FontConfigs::print() const
{
	std::cout << "\nFont\n"
		<< " Relative File Path: " << fontFile << "\n"
		<< " Size: " << fontSize << "pt " << "\n"
		<< " Fill Color:" 
		<< " " << fillColor.r
		<< " " << fillColor.g
		<< " " << fillColor.b << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void PlayerConfigs::print() const
{
	std::cout << "\nPlayer\n"
		<< " Shape Radius: " << shapeRadius << "px\n"
		<< " Calculated Collision Radius: " << collisionRadius << "px\n"
		<< " Lives: " << lives << "\n"
		<< " Speed: " << speed << "\n"
		<< " Fill Color:" 
		<< " " << fillColor.r
		<< " " << fillColor.g
		<< " " << fillColor.b << "\n"
		<< " Outline Color:"
		<< " " << outlineColor.r
		<< " " << outlineColor.g
		<< " " << outlineColor.b << "\n"
		<< " Outline Thickness:" << " " << outlinePx << "px\n"
		<< " Vertices: " << vertices << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void EnemyConfigs::print() const
{
	std::cout << "\nEnemy\n"
		<< " Shape Radius: " << shapeRadius << "px\n"
		<< " Calculated Collision Radius: " << collisionRadius << "px\n"
		<< " Point Value: " << scoreValue << " points per vertex\n"
		<< " Random Vertex Count:"
		<< " " << minVertices << " - " << maxVertices << "\n"
		<< " Random Speed Range:"
		<< " " << minSpeed << " - " << maxSpeed << "\n"
		<< " Enemy Fill Color is random\n"
		<< " Outline Color:"
		<< " " << outlineColor.r
		<< " " << outlineColor.g
		<< " " << outlineColor.b << "\n"
		<< " Outline Thickness:"
		<< " " << outlinePx << "px\n"
		<< " Small Enemy Lifespan: " << smallLifespan << " frames\n"
		<< " Calculated Enemy Lifespan: " 
		<< smallLifespan * 2 << " frames\n"
		<< " Enemy Spawn Interval: " << spawnInterval << " frames\n";
}

////////////////////////////////////////////////////////////////////////////////
void BulletConfigs::print() const
{
	std::cout << "\nBullet\n"
		<< " Shape Radius: " << shapeRadius << "px\n"
		<< " Calculated Collision Radius: " << collisionRadius << "px\n"
		<< " Speed: " << speed << "\n"
		<< " Fill Color:"
		<< " " << fillColor.r
		<< " " << fillColor.g
		<< " " << fillColor.b << "\n"
		<< " Outline Color:"
		<< " " << outlineColor.r
		<< " " << outlineColor.g
		<< " " << outlineColor.b << "\n"
		<< " Outline Thickness: " << outlinePx << "px\n"
		<< " Lifespan: " << lifespan << " frames\n";
}
