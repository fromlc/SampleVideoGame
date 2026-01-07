//------------------------------------------------------------------------------
// main.cpp
// 
// Author: Dr. David Churchill (dchurchill at mun.ca)
// Modified by: L. Carver
// 
// This code is derived from Dr. Churchill's COMP 4300 Assignment 2,
// presented on YouTube: https://youtu.be/-ugbLQlw_VM?si=tZnH4CMx3jueOFG5
// See README file for details.
//------------------------------------------------------------------------------
#include "Game.h"

// globals
////////////////////////////////////////////////////////////////////////////////
namespace GAMEFILES
{
	const std::string cfgFile = "config/config.txt";
	const std::string fontFile = "fonts/Share-TechMono.ttf";
}

// entry point
////////////////////////////////////////////////////////////////////////////////
int main()
{
	// load configs from file
	ConfigsManager cMgr(GAMEFILES::cfgFile);

	// EntityManager and Game both need window and Entity configs
	Configs& cfg = cMgr.getConfigs();

	EntityManager eMgr(cfg);

	sf::Font font(cfg.font.fontFile);
	sf::Text scoreText(font);

	Game g(eMgr, cfg, font, scoreText);
	g.run();

	return 0;
}
