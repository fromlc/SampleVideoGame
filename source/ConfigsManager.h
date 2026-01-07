//------------------------------------------------------------------------------
// ConfigsManager.h
//------------------------------------------------------------------------------
#pragma once

#include <SFML/Graphics.hpp>

//------------------------------------------------------------------------------
// class Configs
//	- config file data read in to contained structs
//	- add config structs to EntityConfigs as needed
//  - struct data types match SFML and ImGui argument lists
//------------------------------------------------------------------------------

// constants
// extra width for ImGui control tabs
constexpr unsigned int containingSizeX = 400;

// Window configs line: width, height, FrameLimit, boolFullScreen
////////////////////////////////////////////////////////////////////////////////
struct WindowConfigs
{
    std::string title;
    // game frame window
    unsigned int frameWidth, frameHeight, frameLimit;
	bool fullScreen;
    // containing window
    unsigned int width, height;

    void print() const;
};

// Font configs line: FontFilename, FontSize, FillColor(R, G, B)
////////////////////////////////////////////////////////////////////////////////
struct FontConfigs
{
	std::string fontFile, fontName;
    int fontSize = 0;
    sf::Color fillColor;

    void print() const;
};

// Player configs line: ShapeRadius, Speed, lives, FillColor(R G B),
//		OutlineColor(R G B), OutlinePixels, ShapeVertices
////////////////////////////////////////////////////////////////////////////////
struct PlayerConfigs
{
    // collisionRadius is not from configs, we provide it
	float shapeRadius, collisionRadius, speed;
    unsigned int lives;
    sf::Color fillColor, outlineColor;
    float outlinePx;
    // SFML uses size_t for vertices
    size_t vertices;

    void print() const;
};

// Enemy gets random fill color, speed, number of vertices
// Enemy configs line: ShapeRadius, PointValue, MinSpeed, MaxSpeed,
//      OutlineColor(R G B), OutlinePixels, MinShapeVertices, MaxShapeVertices
////////////////////////////////////////////////////////////////////////////////
struct EnemyConfigs
{
    // collisionRadius is not from config file, we provide it
    float shapeRadius, collisionRadius;
    unsigned int scoreValue;
    float minSpeed, maxSpeed;
    sf::Color outlineColor;
    float outlinePx;
    // SFML uses size_t for vertices
    size_t minVertices, maxVertices;
    //unsigned int smallLifespan;
    // int for ImGui::SliderInt()
    int smallLifespan;
    //unsigned int spawnInterval;
    // int for ImGui::SliderInt()
    int spawnInterval;
    //unsigned int spawnCount;
    // int for ImGui::SliderInt()
    int spawnCount;

    void print() const;
};

// Bullet configs line: ShapeRadius, CollisionRadius, Speed, 
//      FillColor(R G B), OutlineColor(R G B), OutlinePixels,
//      ShapeVertices, Lifespan
////////////////////////////////////////////////////////////////////////////////
struct BulletConfigs
{
	float shapeRadius, collisionRadius, speed;
    sf::Color fillColor, outlineColor;
    float outlinePx;
    size_t vertices;
    unsigned int lifespan;

    void print() const;
};

////////////////////////////////////////////////////////////////////////////////
struct Configs
{
    WindowConfigs window{};
    FontConfigs   font{};
    PlayerConfigs player{};
    EnemyConfigs  enemy{};
    BulletConfigs bullet{};
};

// containing class
////////////////////////////////////////////////////////////////////////////////
class ConfigsManager
{
public:
    //------------------------------------------------------------------------------
    // constructor loads game data from configuration
    // file into boilerplate Entity Component structs
    //------------------------------------------------------------------------------
    ConfigsManager(const std::string& cfgFile);

    // Game and EntityManager constructors need configs
    Configs& getConfigs();

private:
    Configs m_configs;

    void loadWindowConfigs(std::ifstream& fin);
    void loadFontConfigs(std::ifstream& fin);
    void loadPlayerConfigs(std::ifstream& fin);
    void loadEnemyConfigs(std::ifstream& fin);
    void loadBulletConfigs(std::ifstream& fin);
    void eatCommentLine(std::ifstream& fin);
};
