//------------------------------------------------------------------------------
// DevPanel.hpp
// 
// items editable with ImGui Dev Panel
//------------------------------------------------------------------------------
#pragma once

class DevPanel
{
public:
    bool    runSCollisions = true;      // collision system on
    bool    runSEnemySpawner = true;    // auto spawning on
    bool    manualSpawn = false;
    bool    runSLifespan = true;        // lifespan system on
    bool    runSMovement = true;        // movement system on
    bool    showImGuiDemo = false;      // demo window not visible
    bool    zenMode = false;            // no player collisions
    bool    autoFire = false;
    int	    autoFireFrames = 5;
    bool    autoPlayer = false;
    bool    singleFrameMode = false;
    bool    singleFrameAdvance = false;
    bool    demoMode = false;

    DevPanel() = default;
};

