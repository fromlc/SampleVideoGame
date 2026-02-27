//------------------------------------------------------------------------------
// main.cpp
// 
// Author: Dr. David Churchill (dchurchill at mun.ca)
// Modified by: L. Carver
// 
// This code is derived from Dr. Churchill's COMP 4300 Assignment 2,
// presented on YouTube: https://youtu.be/-ugbLQlw_VM?si=tZnH4CMx3jueOFG5
// See README file for details.
// 
// A big shout-out to all the devs contributing to SFML, Dear ImGui,
// and ImGui-SFML! Thanks for being the heart of many cool projects.
// 
// SFML
//		GitHub: https://github.com/SFML/SFML.git
//		License: https://www.sfml-dev.org/license/
// 
// Dear ImGui
//		GitHub: https://github.com/ocornut/imgui.git
//		License: https://www.dearimgui.com/licenses/
// 
// ImGui-SFML
//		GitHub: https://github.com/SFML/imgui-sfml.git
//		License: https://github.com/SFML/imgui-sfml/blob/master/LICENSE
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
