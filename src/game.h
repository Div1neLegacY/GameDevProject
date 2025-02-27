#pragma once

#include "input.h"
#include "schnitzel_lib.h"
#include "sound.h"
#include "render_interface.h"
#include "ui.h"

#include <string>
#include <functional>  // To use std::function

// #############################################################################
//                           Game Globals
// #############################################################################
constexpr int UPDATES_PER_SECOND = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
// Trying to keep dimensions at a common value for all resolutions
// 640 x 360
constexpr int WORLD_WIDTH = 640;
constexpr int WORLD_HEIGHT = 360;
constexpr int TILESIZE = 95;
constexpr int NUM_OF_TILE_ROWS = 7;
constexpr int NUM_OF_TILE_COLUMNS = 9;
constexpr int GRID_RADIUS = 5;
constexpr IVec2 WORLD_GRID = {NUM_OF_TILE_COLUMNS, NUM_OF_TILE_ROWS};

// #############################################################################
//                           Game Structs
// #############################################################################
enum GameInputType
{
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  ATTACK,

  MOUSE_LEFT,
  MOUSE_RIGHT,

  PAUSE,

  GAME_INPUT_COUNT
};

struct KeyMapping
{
  Array<KeyCodeID, 3> keys;
};

struct Tile
{
  int neighbourMask;
  bool isVisible;
}; 

enum PlayerAnimState
{
  PLAYER_ANIM_IDLE,
  PLAYER_ANIM_RUN,

  PLAYER_ANIM_COUNT
};

struct Player
{
  IVec2 pos;
  IVec2 prevPos;
  Vec2 speed;
  Vec2 solidSpeed;
  int renderOptions;
  float runAnimTime;
  PlayerAnimState animationState;
  SpriteID animationSprites[PLAYER_ANIM_COUNT];
};

struct Solid
{
  SpriteID spriteID;
  IVec2 pos;
  IVec2 prevPos;
  Vec2 remainder;
  Vec2 speed;
  int keyframeIdx;
  Array<IVec2, 2> keyframes;
};

enum GameStateID
{
  GAME_STATE_MAIN_MENU,
  GAME_STATE_IN_LEVEL_1,
  GAME_STATE_IN_LEVEL_2,
};

struct GameState
{
  GameStateID state;
  float updateTimer;
  bool initialized = false;

  Player player;
  // Level 1 Solids
  Array<Solid, 20> solidsLevel1;
  Array<IRect, NUM_OF_TILE_ROWS * NUM_OF_TILE_COLUMNS> backgroundTiles;

  // Level 1 Enemies
  Array<Solid, 5> enemiesLevel1;

  // Level 2 Solids
  Array<Solid, 20> solidsLevel2;
  
  Array<Solid, 5> projectiles;

  Array<IVec2, 21> tileCoords;
  Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
  KeyMapping keyMappings[GAME_INPUT_COUNT];
};

// #############################################################################
//                           Game Globals
// #############################################################################
static GameState* gameState;

// Callbacks
std::function<void(const std::string&)> switchAtlasCallback;

// #############################################################################
//                           Game Functions (Exposed)
// #############################################################################
extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, 
                             RenderData* renderDataIn, 
                             Input* inputIn, 
                             SoundState* soundStateIn,
                             UIState* uiStateIn,
                             float dt);

  EXPORT_FN void game_init(std::function<void(const std::string&)> callback);
}

/**
 * Updates player mechanics.
 * enabledGravity - Allows gravity to affect player
 */
void static update_player(float dt, bool enabledFriction = true, bool enabledGravity = true, bool enabledGrounded = false);

void static update_solids_level_1(float dt);

void static update_solids_level_2(float dt);

void static update_solids(float dt);

void static update_projectiles(float dt);