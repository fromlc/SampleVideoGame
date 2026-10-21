//------------------------------------------------------------------------------
// DevPanel.h
// 
// items editable with ImGui Dev Panel
//------------------------------------------------------------------------------
#pragma once

#include "ConfigsManager.h"
#include "EntityManager.h"

#include <imgui.h>

class DevPanel
{
public:
    bool    runSCollisions;      // collision system on
    bool    runSEnemySpawner;    // auto spawning on
    bool    manualSpawn;
    bool    runSLifespan;        // lifespan system on
    bool    runSMovement;        // movement system on
    bool    showImGuiDemo;        // demo window not visible
    bool    zenMode;              // no player collisions
    bool    autoFire;
    int	    autoFireFrames;
    bool    autoPlayer;
    bool    singleFrameMode;
    bool    singleFrameAdvance;
    bool    demoMode;

    DevPanel();
    void showDevPanel(Entity& player, Configs& cfg, EntityManager& emgr, sf::Vector2u frameSize);

private:
    inline void sfColorToImVec4(sf::Color sfColor, ImVec4& v4Color);
    void playerButton(Entity& player);
    void entityButtons(EntityManager& emgr, ETag tag);
};
