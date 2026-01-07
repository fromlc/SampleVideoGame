//------------------------------------------------------------------------------
// Game.cpp
//------------------------------------------------------------------------------
#include "Game.h"

#include <iostream>
#include <memory>

// draw one-frame explosion on player collision 
constexpr int EXPLOSION_MULTIPLIER = 16;
constexpr int EXPLOSION_CIRCLE_POINTS = 20;

////////////////////////////////////////////////////////////////////////////////
Game::Game(EntityManager& eMgr, Configs& cfg, sf::Font& font, sf::Text& text)
	: m_eMgr(eMgr)
	, m_cfg(cfg)
	, m_font(font)
	, m_text(text)
{
	m_currentFrame = 0;
	m_lastEnemySpawnTime = 0;

	m_paused = false;
	m_gameOver = false;

	// ImGui System tab controls
	m_runSCollisions = true;
	m_runSEnemySpawner = true;
	m_manualSpawn = false;
	m_runSLifespan = true;
	m_runSMovement = true;
	m_showImGuiDemo = false;

	// seed random number generator
	srand((unsigned int)time(0));

	initWindow();
	initScoreText();
	initPlayer();
}

////////////////////////////////////////////////////////////////////////////////
inline void Game::initWindow()
{
	m_window.create(
		sf::VideoMode({ m_cfg.window.width, m_cfg.window.height })
		, "CS 281 Sample Video Game");
	
	m_window.setFramerateLimit(m_cfg.window.frameLimit);
	m_window.setKeyRepeatEnabled(false);

	if (!ImGui::SFML::Init(m_window))
	{
		std::cerr << "Init window failed\n";
		exit(ERRCODE::IMGUI_SFML_INIT_FAIL);
	}

	// ImGui i/o state
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	m_pImguiFont = io.Fonts->AddFontFromFileTTF(
		"fonts/hubballi-regular.ttf", 24);
	if (!ImGui::SFML::UpdateFontTexture())
	{
		std::cerr << "ImGui font not loaded\n";
		exit(ERRCODE::FONT_NOT_LOADED);
	}

	// game frame window
	m_frameSize = { m_cfg.window.frameWidth, m_cfg.window.frameHeight };

	// reset player position on enemy collision
	float x = m_frameSize.x / 2.0f;
	float y = (float)m_frameSize.y;
	m_playerStartPos = { x, y };

	// for positioning text
	m_frameMiddle = { x, y / 2.0f };
}

////////////////////////////////////////////////////////////////////////////////
void Game::restart()
{
	m_eMgr.removeAllEntities();

	CLifespan& rLife = m_player.get<CLifespan>();
	rLife.remaining = rLife.total;

	m_player.get<CTransform>().pos = m_playerStartPos;

	m_player.setScoreValue(0);
	m_gameOver = false;
	m_paused = false;

	m_currentFrame = 0;
}

////////////////////////////////////////////////////////////////////////////////
void Game::doPause() { m_paused = true; }

////////////////////////////////////////////////////////////////////////////////
void Game::unPause() { m_paused = false; }

////////////////////////////////////////////////////////////////////////////////
void Game::sMovePlayer()
{
	// use player pointer, e points to player too
	CInput& rInput = m_player.get<CInput>();

	// player movement requires user input
	if (!rInput.right && !rInput.left)
		return;

	CTransform& rTrans = m_player.get<CTransform>();
	float radius = m_player.get<CCollision>().radius;

	// player at left or right window edge?
	if (rInput.right && rTrans.pos.x + radius < m_frameSize.x)
		rTrans.pos.x += rTrans.velocity.x;

	else if (rInput.left && rTrans.pos.x - radius > 0)
		rTrans.pos.x -= rTrans.velocity.x;
}

// System: Entity position / movement update
//
//	- implements all Entity movement
//	- changes CTransform Component's x, y positions
//	- reverses enemy CTransform Component's velocity when at window edge
////////////////////////////////////////////////////////////////////////////////
void Game::sMovement()
{
	// ImGui System tab control
	if (!m_runSMovement) { return; }

	// player is special case, process before loop
	sMovePlayer();

	// move all other Entities
	for (auto& kv : m_eMgr.getEntityMap())
	{
		// key is Entity type char (e=enemy, b=bullet etc)
		ETag tag = kv.first;

		// value is std::vector<spEntity>
		for (spEntity e : kv.second)
		{
			CTransform& rTrans = e->get<CTransform>();

			// only Enemy bounces off window edges
			if (ETag::ENEMY == tag)
			{
				unsigned int distToEdge = (unsigned int)e->get<CCollision>().radius;

				// at right or left edge?
				if (rTrans.pos.x + distToEdge * 2 >= m_frameSize.x
					|| rTrans.pos.x - distToEdge <= 0)
					rTrans.velocity.x = (-rTrans.velocity.x);

				// at bottom or top edge?
				if (rTrans.pos.y + distToEdge >= m_frameSize.y
					|| rTrans.pos.y - distToEdge <= 0)
					rTrans.velocity.y = (-rTrans.velocity.y);
			}
			// move
			rTrans.pos += rTrans.velocity;
		}
	}
}

// System: User Input
////////////////////////////////////////////////////////////////////////////////
void Game::sKeyPressed(sf::Keyboard::Scancode keyPrs)
{
	CInput& rInput = m_player.get<CInput>();

	switch (keyPrs)
	{
	case sf::Keyboard::Scan::Escape:
		m_window.close();
		exit(ERRCODE::USER_EXIT);

	case sf::Keyboard::Scan::Space:
	case sf::Keyboard::Scan::Pause:
		m_paused = m_gameOver ? true : !m_paused; break;

	case sf::Keyboard::Scan::F5:    restart(); break;

	case sf::Keyboard::Scan::Left:  rInput.left = true; break;
	case sf::Keyboard::Scan::Right: rInput.right = true; break;

	case sf::Keyboard::Scan::S:
	case sf::Keyboard::Scan::D:
	case sf::Keyboard::Scan::F:
		rInput.shoot = true;
		m_eMgr.spawnBullet(m_player.pos());
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
void Game::sKeyReleased(sf::Keyboard::Scancode keyRel)
{
	CInput& rInput = m_player.get<CInput>();

	switch (keyRel)
	{
	// S, D, F keys all shoot bullets
	case sf::Keyboard::Scan::S:
	case sf::Keyboard::Scan::D:
	case sf::Keyboard::Scan::F:		rInput.shoot = false; break;

	case sf::Keyboard::Scan::Left:  rInput.left = false; break;
	case sf::Keyboard::Scan::Right:	rInput.right = false; break;
	}
}

////////////////////////////////////////////////////////////////////////////////
void Game::sUserInput()
{
	// prevent background interaction when ImGui is using mouse/keyboard
	//ImGuiIO& io = ImGui::GetIO();
	//if (io.WantCaptureMouse || io.WantCaptureKeyboard)
	//	return;

	while (const std::optional event = m_window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(m_window, *event);

		if (event->is<sf::Event::Closed>())
		{
			m_window.close();
			exit(ERRCODE::USER_EXIT);
		}

		// catch resize events
		if (const auto* resized = event->getIf<sf::Event::Resized>())
		{
			// update the view to new window size
			sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(resized->size));
			m_window.setView(sf::View(visibleArea));
		}

		if (const auto* pKeyP = event->getIf<sf::Event::KeyPressed>())
			sKeyPressed(pKeyP->scancode);

		else if (const auto* pKeyR = event->getIf<sf::Event::KeyReleased>())
			sKeyReleased(pKeyR->scancode);
	}
}

// System: Lifespan
//	- all Entities in map have CLifespan Component
//	- if entity has 0 lifespan remaining, destroy it
//	- else subtract 1 and fade alpha channel opacity
// key kv.first is Entity ETag (ENEMY, BULLET etc.)
// value kv.second is std::vector<spEntity>
////////////////////////////////////////////////////////////////////////////////
void Game::sLifespan()
{
	// ImGui System tab control
	if (!m_runSLifespan) { return; }

	for (auto& kv : m_eMgr.getEntityMap())
	{
		for (spEntity e : kv.second)
		{
			CLifespan& rLife = e->get<CLifespan>();

			if (1 == rLife.remaining--)
				e->selfDestruct();
			else
			{
				// fade entity opacity over lifespan
				sf::Color color = e->get<CShape>().circle.getFillColor();
				//#TODO fade color more slowly
				color.a = (uint8_t)(rLife.remaining / ((float)rLife.total) * 255);
				e->get<CShape>().circle.setFillColor(color);
			}
		}
		m_eMgr.removeDeadEntities(kv.first);
	}
}

// System: Spawns Enemies
////////////////////////////////////////////////////////////////////////////////
void Game::sEnemySpawner()
{
	// ImGui System tab controls
	if (!m_runSEnemySpawner && !m_manualSpawn) { return; }

	if ((m_currentFrame - m_lastEnemySpawnTime)
		>= m_cfg.enemy.spawnInterval)
	{
		m_eMgr.spawnEnemy(m_frameSize);

		m_lastEnemySpawnTime = m_currentFrame;
	}
}

// System: Detects collisions
////////////////////////////////////////////////////////////////////////////////
void Game::sCollisions()
{
	// ImGui System tab controls
	if (!m_runSCollisions) { return; }

	// configs data does not change during Game
	static float bcRadius = m_cfg.bullet.collisionRadius;
	static float ecRadius = m_cfg.enemy.collisionRadius;
	//#TODO make sure this is not floating point division
	static float secRadius = ecRadius / 2;

	// bullet/smallEnemy collision distances
	static float becDistance = bcRadius + ecRadius;
	static float bsecDistance = bcRadius + secRadius;

	// enemy/player collision distance
	static float pcRadius = m_cfg.player.collisionRadius;
	static float epcDistance = ecRadius + pcRadius;

	// smallEnemy/player collision distance
	static float sepcDistance = secRadius + pcRadius;

	// 1) check for enemy collision with player

	sf::Vector2f playerPos = m_player.pos();

	for (spEntity e : m_eMgr.getEntities(ETag::ENEMY))
	{
		if (e->distanceTo(playerPos) <= epcDistance)
		{
			// deduct points and set player crash color
			damagePlayer(e->getScoreValue());
			m_player.get<CShape>().circle.setOutlineColor(m_cfg.player.outlineColor);
			m_player.get<CShape>().circle.setOutlineThickness(
				m_cfg.player.outlinePx * EXPLOSION_MULTIPLIER);
			m_player.get<CShape>().circle.setPointCount(
				EXPLOSION_CIRCLE_POINTS);

			e->selfDestruct();
		}
	}

	// prevent dead Entities from colliding anymore
	m_eMgr.removeDeadEntities(ETag::ENEMY);

	// 2) check for smallEnemy collisions with player

	for (spEntity se : m_eMgr.getEntities(ETag::SMALLENEMY))
	{
		if (se->distanceTo(playerPos) <= sepcDistance)
		{
			// deduct points and set player crash color
			damagePlayer(se->getScoreValue());
			m_player.get<CShape>().circle.setOutlineColor({ 255, 0, 0 });
			m_player.get<CShape>().circle.setOutlineThickness(m_cfg.player.outlinePx * 12);
			m_player.get<CShape>().circle.setPointCount(30);
			se->selfDestruct();
		}
	}

	// prevent dead Entities from colliding anymore
	m_eMgr.removeDeadEntities(ETag::SMALLENEMY);

	// 3) check for bullet collisions with enemy

	for ( spEntity b : m_eMgr.getEntities(ETag::BULLET))
	{
		for (spEntity e : m_eMgr.getEntities(ETag::ENEMY))
		{
			if (b->distanceTo(e->pos()) <= becDistance)
			{
				b->selfDestruct();

				m_player.addPoints(e->getScoreValue());
				m_eMgr.spawnSmallEnemies(e);
				e->selfDestruct();
			}
		}
	}

	// prevent dead Entities from colliding anymore
	m_eMgr.removeDeadEntities(ETag::BULLET);
	m_eMgr.removeDeadEntities(ETag::ENEMY);

	// 3) check for bullet collisions with smallEnemy 
	// need a new iterator for bullet vector after erasing dead

	for (spEntity b : m_eMgr.getEntities(ETag::BULLET))
	{
		for (spEntity se : m_eMgr.getEntities(ETag::SMALLENEMY))
		{
			if (b->distanceTo(se->pos()) <= bsecDistance)
			{
				b->selfDestruct();

				m_player.addPoints(se->getScoreValue());
				se->selfDestruct();
			}
		}
	}

	// prevent dead Entities from colliding anymore
	m_eMgr.removeDeadEntities(ETag::BULLET);
	m_eMgr.removeDeadEntities(ETag::SMALLENEMY);
}

////////////////////////////////////////////////////////////////////////////////
inline void Game::initPlayer()
{
	m_player.setScoreValue(0);

	// player centered with angle = 0
	m_player.add<CTransform>(
		{ m_playerStartPos
		, { m_cfg.player.speed, 0 }
		, 0.0f
		});

	m_player.add<CShape>(
		{ m_cfg.player.shapeRadius
		, m_cfg.player.vertices
		, m_cfg.player.fillColor
		, m_cfg.player.outlineColor
		, m_cfg.player.outlinePx
		});

	// calculate and set collision radius
	m_player.add<CCollision>(
		{ m_cfg.player.shapeRadius + m_cfg.player.outlinePx });

	// player movement/shooting controlled by user inputs
	m_player.add<CInput>({ false, false, false });

	m_player.add<CLifespan>(m_cfg.player.lives);

	//#TODO implement special weapon
	//m_player.add<CSpecialWeapon>({});
}

// adjust player score and reset player screen position after collision
////////////////////////////////////////////////////////////////////////////////
void Game::damagePlayer(int losePoints)
{
	m_player.addPoints(-losePoints);

	CLifespan& rLife = m_player.get<CLifespan>();

	// freeze with only UserInput on player lives gone
	if (1 == rLife.remaining--)
		m_gameOver = true;
	//else
	//	m_player.get<CTransform>().pos = m_playerStartPos;
}

// System: ImGui
////////////////////////////////////////////////////////////////////////////////
namespace IMGUIID { static int id = 0; }

////////////////////////////////////////////////////////////////////////////////
inline void Game::sfColorToImVec4(sf::Color sfColor, ImVec4& v4Color)
{
	v4Color.x = sfColor.r / 255.0f;
	v4Color.y = sfColor.g / 255.0f;
	v4Color.z = sfColor.b / 255.0f;
	v4Color.w = sfColor.a / 255.0f;
}

////////////////////////////////////////////////////////////////////////////////
void Game::sPlayerButton()
{
	// use player outline color
	ImVec4 buttonColor;
	sfColorToImVec4(m_player.get<CShape>().circle.getOutlineColor(), buttonColor);

	ImGui::PushID(IMGUIID::id++);
	ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
	ImGui::Button(" ");

	ImGui::PopStyleColor(3);
	ImGui::PopID();

	ImGui::SameLine();
	sf::Vector2f playerPos = m_player.pos();
	ImGui::Text("player %d at (%.0f,%.0f) life %i"
		, m_player.getId(), playerPos.x, playerPos.y
		, m_player.get<CLifespan>().remaining);
}

////////////////////////////////////////////////////////////////////////////////
void Game::sEntityButtons(ETag tag)
{
	for (spEntity e : m_eMgr.getEntities(tag))
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
void Game::showDevPanel()
{
	// specify default position/size in case there's no .ini file data
	// from imgui_demo.cpp:414
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(450, 680), ImGuiCond_FirstUseEver);

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
			ImGui::Checkbox("Movement", &m_runSMovement);
			ImGui::Checkbox("Lifespan", &m_runSLifespan);
			ImGui::Checkbox("Collision", &m_runSCollisions);
			ImGui::Checkbox("Spawning", &m_runSEnemySpawner);
			ImGui::Checkbox("Dear ImGui Demo", &m_showImGuiDemo);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Game"))
		{
			//#TODO for loop on EntityMap keys
			if (ImGui::CollapsingHeader("Enemies", ImGuiTreeNodeFlags_None))
				sEntityButtons(ETag::ENEMY);

			if (ImGui::CollapsingHeader("Bullets", ImGuiTreeNodeFlags_None))
				sEntityButtons(ETag::BULLET);

			if (ImGui::CollapsingHeader("Small Enemies", ImGuiTreeNodeFlags_None))
				sEntityButtons(ETag::SMALLENEMY);

			if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_None))
				sPlayerButton();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Controls"))
		{
			if (ImGui::CollapsingHeader("Spawning", ImGuiTreeNodeFlags_None))
			{
				ImGui::SliderInt("Time Interval"
					, &m_cfg.enemy.spawnInterval
					, 10
					, 90);

				ImGui::SliderInt("Enemy Count"
					, &m_cfg.enemy.spawnCount
					, 1
					, 10);

				ImGui::Separator();

				if (ImGui::Button("Spawn One Enemy"))
				{
					m_manualSpawn = true;
					m_eMgr.spawnEnemy(m_frameSize);
					m_manualSpawn = false;

					ImGui::SameLine();
					ImGui::Text("Spawned!");
				}
				ImGui::SameLine();
				ImGui::Button("Spawn One Bullet");
				if (ImGui::IsItemActive())
				{
					m_manualSpawn = true;
					m_eMgr.spawnBullet(m_player.pos());
					m_manualSpawn = false;

					ImGui::SameLine();
					ImGui::Text("Spawned!");
				}
				ImGui::Separator();
			}
			if (ImGui::CollapsingHeader("Enemies", ImGuiTreeNodeFlags_None))
			{
				ImGui::SliderFloat("Min Speed"
					, &m_cfg.enemy.minSpeed
					, 1
					, 50.0);

				ImGui::SliderFloat("Max Speed"
					, &m_cfg.enemy.maxSpeed
					, 1
					, 50.0);

				if (m_cfg.enemy.minSpeed > m_cfg.enemy.maxSpeed)
					m_cfg.enemy.minSpeed = m_cfg.enemy.maxSpeed;

				ImGui::PushID(IMGUIID::id++);	// duplicate name below
				ImGui::SliderInt("Lifespan"
					, &m_cfg.enemy.smallLifespan
					, 10, 1000);

				ImGui::PopID();
				ImGui::Separator();
			}
			if (ImGui::CollapsingHeader("Bullets", ImGuiTreeNodeFlags_None))
			{
				ImGui::PushID(IMGUIID::id++);
				ImGui::SliderFloat("Speed"
					, &m_cfg.bullet.speed
					, m_cfg.enemy.minSpeed
					, 50.0);
				ImGui::PopID();

				ImGui::PushID(IMGUIID::id++);
				static int bulletLife = (int)m_cfg.bullet.lifespan;
				ImGui::SliderInt("Lifespan", &bulletLife, 10, 1000);
				m_cfg.bullet.lifespan = bulletLife;
				ImGui::PopID();
				ImGui::Separator();
			}
			if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_None))
			{
				ImGui::PushID(IMGUIID::id++);
				ImGui::SliderFloat("Speed",
					&m_player.get<CTransform>().velocity.x
					, m_cfg.enemy.minSpeed
					, 30.0);
				ImGui::PopID();
				ImGui::Separator();
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("About"))
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
				ImGui::BulletText("Enemies are worth 10 points per vertex.");
				ImGui::BulletText("Small enemies are worth 2X spawning enemy.");
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
		ImGui::EndTabBar();
		//ImGui::PopItemWidth();
	}
	ImGui::End();
}

// System: Render / Drawing
////////////////////////////////////////////////////////////////////////////////
void Game::sRenderPlayer(sf::RenderTexture& rTexture)
{
	// set shape position based on Entity's transform->pos
	m_player.get<CShape>().circle.setPosition(m_player.pos());

	// set shape position based on Entity's transform->angle
	m_player.get<CTransform>().angle += 1.0f;
	m_player.get<CShape>().circle.setRotation(
		sf::degrees(m_player.get<CTransform>().angle));

	// draw the entity's sf::CircleShape
	rTexture.draw(m_player.get<CShape>().circle);

	//#TODO assume collision -> reset player appearance
	m_player.get<CShape>().circle.setOutlineColor(m_cfg.player.outlineColor);
	m_player.get<CShape>().circle.setOutlineThickness(m_cfg.player.outlinePx);
	m_player.get<CShape>().circle.setPointCount(m_cfg.player.vertices);
}

////////////////////////////////////////////////////////////////////////////////
void Game::sRenderEntity(spEntity e, sf::RenderTexture& rTexture)
{
	// set shape position base on the entity's transform->pos
	e->get<CShape>().circle.setPosition(e->pos());

	// set shape position base on the entity's transform->angle
	e->get<CTransform>().angle += 1.0f;
	e->get<CShape>().circle.setRotation(
		sf::degrees(e->get<CTransform>().angle));

	// draw the entity's sf::CircleShape
	rTexture.draw(e->get<CShape>().circle);
}

////////////////////////////////////////////////////////////////////////////////
void Game::sRenderScore(sf::RenderTexture& rTexture)
{
	// update score display
	std::string scoreStr = "Score: "
		+ std::to_string(m_player.getScoreValue())
		+ "\n"
		+ "Lives Left: "
		+ std::to_string(m_player.get<CLifespan>().remaining);

	m_text.setString(scoreStr);
	rTexture.draw(m_text);
}

// render all Entities and scoreboard
////////////////////////////////////////////////////////////////////////////////
void Game::sRender()
{
	ImGui::SFML::Update(m_window, m_deltaClock.restart());

	static sf::RenderTexture frameTexture(
		{ (unsigned int)m_frameSize.x, (unsigned int)m_frameSize.y });
	//sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(m_frameSize));
	//m_window.setView(sf::View(visibleArea));

	frameTexture.clear();
	sRenderPlayer(frameTexture);

	for (auto& kv : m_eMgr.getEntityMap())
	{
		// key is Entity ETag (ENEMY, BULLET, etc.)
		// value is std::vector<spEntity>
		for (spEntity e : kv.second)
			sRenderEntity(e, frameTexture);
	}
	sRenderScore(frameTexture);

	if (m_gameOver)
		sRenderGameOver(frameTexture);

	ImGui::PushFont(m_pImguiFont);
	ImGui::DockSpaceOverViewport();

	// Dear ImGui Demo
	if (m_showImGuiDemo)
		ImGui::ShowDemoWindow();

	showDevPanel();

	// specify default position/size in case there's no .ini file data
	// from imgui_demo.cpp:414
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(
		ImVec2(main_viewport->WorkPos.x + 450.0f
			 , main_viewport->WorkPos.y));
	ImGui::SetNextWindowSize(ImVec2(
		(float)m_frameSize.x, (float)m_frameSize.y + 20.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	if (ImGui::Begin("Not A2 Anymore", NULL,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
		ImGui::Image(frameTexture);

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopFont();

	m_window.clear();
	ImGui::SFML::Render(m_window);
	m_window.display();
}

////////////////////////////////////////////////////////////////////////////////
inline void Game::initScoreText()
{
	m_text.setFillColor(m_cfg.font.fillColor);

	//#TODO add font outline color to configs
	m_text.setOutlineColor(sf::Color(0, 0, 0));
	m_text.setOutlineThickness(2);
}

////////////////////////////////////////////////////////////////////////////////
void Game::sRenderGameOver(sf::RenderTexture& rTexture)
{
	static sf::Text text1(m_font);
	static sf::Text text2(m_font);

	text1.setFillColor(sf::Color::Yellow);
	text1.setOutlineColor(sf::Color(0, 0, 0));
	text1.setOutlineThickness(2);
	text1.setCharacterSize(64);
	text1.setString("GAME OVER");

	text2.setFillColor(sf::Color::Cyan);
	text2.setOutlineColor(sf::Color(0, 0, 0));
	text2.setOutlineThickness(2);
	text2.setCharacterSize(36);
	text2.setString("(Press F5 to play again!)");

	sf::Vector2f text1Size = text1.getLocalBounds().size;
	float text1YPos = m_frameMiddle.y - (text1Size.y * 2);
	float text1XPos = m_frameMiddle.x - (text1Size.x / 2.0f);
	text1.setPosition({ text1XPos, text1YPos } );

	sf::Vector2f text2Size = text2.getLocalBounds().size;
	float text2XPos = m_frameMiddle.x - (text2Size.x / 2.0f);
	float text2YPos = text1YPos + (text1Size.y * 2);
	text2.setPosition({ text2XPos, text2YPos });
	
	rTexture.draw(text1);
	rTexture.draw(text2);
}

// main game loop
////////////////////////////////////////////////////////////////////////////////
void Game::run()
{
	while (m_window.isOpen())
	{
		while (!m_gameOver)
		{
			if (!m_paused)
			{
				// add waiting Entities
				m_eMgr.updateEntityMap();
				// user input -> movement
				sMovement();
				// movement -> collision -> removal
				sCollisions();
				// end of life -> removal
				sLifespan();
				sEnemySpawner();
			}
			// rotate Entities and process user input in pause mode
			sRender();
			sUserInput();

			m_currentFrame++;
		}
		// always render and process user input
		sRender();
		sUserInput();
	}
	ImGui::SFML::Shutdown();
}
