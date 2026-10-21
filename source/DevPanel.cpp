//------------------------------------------------------------------------------
// DevPanel.cpp
// 
// items editable with ImGui Dev Panel
//------------------------------------------------------------------------------
#pragma once

#include "DevPanel.h"

#include <imgui.h>

////////////////////////////////////////////////////////////////////////////////
namespace IMGUIID { static int id = 0; }

////////////////////////////////////////////////////////////////////////////////
inline void DevPanel::sfColorToImVec4(sf::Color sfColor, ImVec4& v4Color)
{
    v4Color.x = sfColor.r / 255.0f;
    v4Color.y = sfColor.g / 255.0f;
    v4Color.z = sfColor.b / 255.0f;
    v4Color.w = sfColor.a / 255.0f;
}

////////////////////////////////////////////////////////////////////////////////
void DevPanel::playerButton(Entity& player)
{
    // use player outline color
    ImVec4 buttonColor;
    sfColorToImVec4(player.get<CShape>().circle.getOutlineColor(), buttonColor);

    ImGui::PushID(IMGUIID::id++);
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
    ImGui::Button(" ");

    ImGui::PopStyleColor(3);
    ImGui::PopID();

    ImGui::SameLine();
    sf::Vector2f playerPos = player.pos();
    ImGui::Text("player %d at (%.0f,%.0f) life %i"
                , player.getId(), playerPos.x, playerPos.y
                , player.get<CLifespan>().remaining);
}

////////////////////////////////////////////////////////////////////////////////
void DevPanel::entityButtons(EntityManager& emgr, ETag tag)
{
    for (spEntity e : emgr.getEntities(tag))
    {
        ImVec4 buttonColor, hoverColor, activeColor;
        sfColorToImVec4(e->get<CShape>().circle.getFillColor(), buttonColor);
        sfColorToImVec4({ 100, 100, 100 }, hoverColor);
        sfColorToImVec4({ 10, 10, 10 }, activeColor);

        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
        ImGui::PushID(IMGUIID::id++);
        ImGui::Button("K");

        // click button to kill this Entity
        if (ImGui::IsItemActive())
            e->selfDestruct();

        ImGui::PopID();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        sf::Vector2f velocity = e->get<CTransform>().velocity;
        ImGui::Text("id %d", e->getId());

        // spawned Entity displays spawner's id
        if (e->has<CMeta>())
        {
            ImGui::SameLine();
            ImGui::Text("(%d)", e->get<CMeta>().spawnerId);
        }

        ImGui::SameLine();
        ImGui::Text("pos(%.0f,%.0f) speed(%0.f,%0.f) life %i"
                    , e->pos().x, e->pos().y
                    , velocity.x, velocity.y
                    , e->get<CLifespan>().remaining);
    }
}

////////////////////////////////////////////////////////////////////////////////
void DevPanel::showDevPanel(Entity& player, Configs& cfg, EntityManager& emgr, sf::Vector2u frameSize)
{
    // specify default position/size in case there's no .ini file data
    // from imgui_demo.cpp:414
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 20)
        , ImGuiCond_FirstUseEver
    );
    // prevent window resize by user
    //ImGui::SetNextWindowSize(
    //    ImVec2(devPanelSizeX, (float)m_cfg.window.frameHeight)
    //    , ImGuiCond_FirstUseEver
    //);
    ImGui::SetNextWindowSize(
        ImVec2(devPanelSizeX, (float)cfg.window.frameHeight)
        , 0
    );

    ImGuiWindowFlags windowFlags = 0;
    //ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoSavedSettings;
    //ImGuiWindowFlags windowFlags = ImGuiCond_Once;
    //ImGui::Begin("Dev Panel", NULL, windowFlags);
    ImGui::Begin("Dev Panel", NULL, windowFlags);
    ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_Reorderable;
    // limit tab bar width
    //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
    if (ImGui::BeginTabBar("ControlTabBar", tabBarFlags))
    {
        if (ImGui::BeginTabItem("Systems"))
        {
            ImGui::Checkbox("Movement", &runSMovement);
            ImGui::Checkbox("Lifespan", &runSLifespan);
            ImGui::Checkbox("Collision", &runSCollisions);
            ImGui::Checkbox("Spawning", &runSEnemySpawner);
            ImGui::Checkbox("Auto Fire", &autoFire);
            if (autoFire)
            {
                ImGui::SliderInt("Frame Interval"
                                 , &autoFireFrames
                                 , 1
                                 , 10
                );
            }
            ImGui::Checkbox("Auto Player Move", &autoPlayer);
            ImGui::Checkbox("Zen Mode (No Player Collisions)", &zenMode);
            ImGui::Checkbox("Single Frame Advance Mode", &singleFrameMode);
            if (singleFrameMode)
            {
                if (singleFrameAdvance = ImGui::Button("Advance One Frame"))
                    singleFrameAdvance = true;
            }
            ImGui::Checkbox("Demo Mode", &demoMode);
            autoPlayer = autoFire = zenMode = demoMode;

            ImGui::SameLine();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            //#TODO for loop on EntityMap keys
            if (ImGui::CollapsingHeader("Enemies", ImGuiTreeNodeFlags_None))
                entityButtons(emgr, ETag::ENEMY);

            if (ImGui::CollapsingHeader("Bullets", ImGuiTreeNodeFlags_None))
                entityButtons(emgr, ETag::BULLET);

            if (ImGui::CollapsingHeader("Small Enemies", ImGuiTreeNodeFlags_None))
                entityButtons(emgr, ETag::SMALLENEMY);

            if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_None))
                playerButton(player);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Controls"))
        {
            if (ImGui::CollapsingHeader("Spawning", ImGuiTreeNodeFlags_None))
            {
                ImGui::SliderInt("Time Interval"
                                 , &cfg.enemy.spawnInterval
                                 , 10
                                 , 90
                );
                ImGui::SliderInt("Enemy Count"
                                 , &cfg.enemy.spawnCount
                                 , 1
                                 , 10
                );
                ImGui::Separator();

                if (ImGui::Button("Spawn Enemies"))
                {
                    manualSpawn = true;
                    emgr.spawnEnemy(frameSize);
                    manualSpawn = false;

                    ImGui::SameLine();
                    ImGui::Text("Spawned!");
                }
                ImGui::SameLine();
                ImGui::Button("Spawn Bullet");
                if (ImGui::IsItemActive())
                {
                    manualSpawn = true;
                    emgr.spawnBullet(player.pos());
                    manualSpawn = false;

                    ImGui::SameLine();
                    ImGui::Text("Spawned!");
                }
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Enemies", ImGuiTreeNodeFlags_None))
            {
                ImGui::SliderFloat("Min Speed"
                                   , &cfg.enemy.minSpeed
                                   , 1.0f
                                   , 50.0f
                                   , "%.1f"
                );
                ImGui::SliderFloat("Max Speed"
                                   , &cfg.enemy.maxSpeed
                                   , 1.0f
                                   , 50.0f
                                   , "%.1f"
                );
                if (cfg.enemy.minSpeed > cfg.enemy.maxSpeed)
                    cfg.enemy.minSpeed = cfg.enemy.maxSpeed;

                ImGui::PushID(IMGUIID::id++);	// duplicate name below
                ImGui::SliderInt("Lifespan"
                                 , &cfg.enemy.smallLifespan
                                 , 10
                                 , 1000
                );
                ImGui::PopID();
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Bullets", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushID(IMGUIID::id++);
                ImGui::SliderFloat("Speed"
                                   , &cfg.bullet.speed
                                   , cfg.enemy.minSpeed
                                   , 50.0f
                                   , "%.1f"
                );
                ImGui::PopID();

                ImGui::PushID(IMGUIID::id++);
                static int bulletLife = (int)cfg.bullet.lifespan;
                ImGui::SliderInt("Lifespan"
                                 , &bulletLife
                                 , 10
                                 , 1000
                );
                cfg.bullet.lifespan = bulletLife;
                ImGui::PopID();
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushID(IMGUIID::id++);
                ImGui::SliderFloat("Speed",
                                   &player.get<CTransform>().velocity.x
                                   , cfg.enemy.minSpeed
                                   , 20.0f
                                   , "%.1f"
                );
                ImGui::PopID();

                ImGui::PushID(IMGUIID::id++);
                ImGui::SliderInt("Total Lives"
                                 , (int*)&player.get<CLifespan>().total
                                 , 1
                                 , 100
                );
                ImGui::PopID();

                ImGui::PushID(IMGUIID::id++);
                ImGui::SliderInt("Lives Left"
                                 , (int*)&player.get<CLifespan>().remaining
                                 , 0
                                 , 100
                );
                ImGui::PopID();
                ImGui::Separator();
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Help"))
        {
            if (ImGui::CollapsingHeader("Keyboard Commands", ImGuiTreeNodeFlags_None))
            {
                ImGui::BulletText("Left and Right Arrow keys move Player.");
                ImGui::BulletText("S, D, and F keys all fire bullets.");
                ImGui::BulletText("Spacebar and Pause keys toggle pause mode.");
                ImGui::BulletText("F5 key restarts game.");
                ImGui::BulletText("Esc key exits game.");
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Player/Enemy Collisions", ImGuiTreeNodeFlags_None))
            {
                ImGui::BulletText("Player loses a life, and");
                ImGui::BulletText("Enemy's point value is deducted from Player.");
                ImGui::BulletText("Enemy spawns N small enemies then dies.");
                ImGui::BulletText("N is spawning enemy's vertex count.");
                ImGui::BulletText("Enemies don't collide with each other.");
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Scoring", ImGuiTreeNodeFlags_None))
            {
                ImGui::BulletText("Enemies: 10 * (#vertices + speed).");
                ImGui::BulletText("Small enemies: 2X spawning enemy.");
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Game Files", ImGuiTreeNodeFlags_None))
            {
                ImGui::BulletText("config/config.txt");
                ImGui::BulletText("fonts/ folder contains TTF font files");
                ImGui::Separator();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ImGui"))
        {
//            ImGui::SameLine();
            ImGui::Text("Dear ImGui is an open-source runtime");
            ImGui::Text("game editor used in game development.");
            ImGui::Text("It's great for debugging and tuning");
            ImGui::Text("game settings for maximum fun.");
            ImGui::Text("The main developer is Omar Cornut,");
            ImGui::Text("with contributions from other skilled");
            ImGui::Text("game developers.");
            ImGui::Separator();

            ImGui::TextLinkOpenURL("Dear ImGui home page", "https://github.com/ocornut/imgui");
            ImGui::TextLinkOpenURL("Dear ImGui FAQs and links", "https://www.dearimgui.com/");
            ImGui::TextLinkOpenURL("Software using Dear ImGui", "https://github.com/ocornut/imgui/wiki/Software-using-Dear-ImGui");
            ImGui::TextLinkOpenURL("Licenses and Sales", "https://www.dearimgui.com/licenses/");
            ImGui::Separator();

            ImGui::Checkbox("Show Dear ImGui Demo Window", &showImGuiDemo);
            ImGui::Separator();

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        //ImGui::PopItemWidth();
    }
    ImGui::End();
}
