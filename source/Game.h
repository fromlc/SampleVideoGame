//------------------------------------------------------------------------------
// Game.h
//------------------------------------------------------------------------------
#pragma once

#include "EntityManager.h"
#include "errors.h"

#ifndef GAME_NO_DEV_IMGUI
#include <imgui.h>
#include <imgui-SFML.h>
#endif

#include <memory>
#include <string>

////////////////////////////////////////////////////////////////////////////////
class Game
{
public:
	Game() = default;
	Game(EntityManager& eMgr, Configs& cfg, sf::Font&, sf::Text&);
	void run();

private:
	Entity				m_player;			// quick player data access
	sf::RenderWindow	m_window;			// containing window
	sf::Vector2f		m_frameMiddle;		// for placing text
	sf::Vector2u		m_frameSize;		// 
	sf::Vector2f		m_playerStartPos;	// for resetting player pos
	sf::Font&			m_font;				// for all text
	sf::Text&			m_text;				// scoreboard text
	ImFont*				m_pImguiFont;		// for ImGui windows

	// ImGui Dev Panel
	bool				m_runSCollisions;
	//#TODO only need one boolean? m_manualSpawn
	bool				m_runSEnemySpawner;
	bool				m_manualSpawn;
	bool				m_runSLifespan;
	bool				m_runSMovement;
	bool				m_showImGuiDemo;

	Configs&			m_cfg;				// loaded from file
	EntityManager&		m_eMgr;				// maintains entities

	size_t				m_lastEnemySpawnTime;
	sf::Clock			m_deltaClock;

	size_t				m_currentFrame;
	bool				m_paused;			// no game logic update on pause
	bool				m_gameOver;			// set true on player lives gone

	inline void initWindow();
	inline void initPlayer();

	void  damagePlayer(int);

	void restart();
	void doPause();
	void unPause();
							// System: Entity position / movement update
	void sMovePlayer();
	void sMovement();
							// System: User Input
	void sKeyPressed(sf::Keyboard::Scancode);
	void sKeyReleased(sf::Keyboard::Scancode);
	void sUserInput();

	void sLifespan();		// System: Lifespan

							// System: ImGui in DEBUG config only
	inline void sfColorToImVec4(sf::Color, ImVec4&);
	void sEntityButtons(ETag);
	void sPlayerButton();
	void showDevPanel();
							// System: Render / Drawing
	void sRenderPlayer(sf::RenderTexture&);	// only player uses stack memory
	void sRenderEntity(spEntity, sf::RenderTexture&);
	void sRenderScore(sf::RenderTexture&);
	void sRenderGameOver(sf::RenderTexture&);
	void sRender();
	
	void sEnemySpawner();	// System: Spawns enemies at config intervals
	void sCollisions();		// System: Collisions

	inline void initScoreText();
};
