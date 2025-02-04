#include "game.h"

#include "assets.h"
#include "texts.h"
#include <cmath>
#include <iostream>

// #############################################################################
//                           Game Constants
// #############################################################################

// #############################################################################
//                           Game Structs
// #############################################################################

// #############################################################################
//                           Game Functions
// #############################################################################
bool just_pressed(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for(int idx = 0; idx < mapping.keys.count; idx++)
  {
    if(input->keys[mapping.keys[idx]].justPressed)
    {
      return true;
    }
  }

  return false;
}

bool is_down(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for(int idx = 0; idx < mapping.keys.count; idx++)
  {
    if(input->keys[mapping.keys[idx]].isDown)
    {
      return true;
    }
  }

  return false;
}

/**
 * Gets the grid index [x, y] of position in world
 * 
 * e.g. starting tile [0, 0], moving over 1 tile to left --> [-1, 0]
 */
IVec2 get_grid_pos(IVec2 worldPos)
{
  int gridPosX;
  int gridPosY;
  if (worldPos.x > 0)
  {
    gridPosX = (worldPos.x + (TILESIZE / 2)) / TILESIZE;
  }
  else
  {
    gridPosX = (worldPos.x - (TILESIZE / 2)) / TILESIZE;
  }

  if (worldPos.y > 0)
  {
    gridPosY = (worldPos.y + (TILESIZE / 2)) / TILESIZE;
  }
  else
  {
    gridPosY = (worldPos.y - (TILESIZE / 2)) / TILESIZE;
  }

  return {gridPosX, gridPosY};
}


Tile* get_tile(int x, int y)
{
  Tile* tile = nullptr;

  if(x >= 0  && x < WORLD_GRID.x && y >= 0 && y < WORLD_GRID.y)
  {
    tile = &gameState->worldGrid[x][y];
  }

  return tile;
}

Tile* get_tile(IVec2 worldPos)
{
  IVec2 gridPos = get_grid_pos(worldPos);
  return get_tile(gridPos.x, gridPos.y);
}

IRect get_player_rect()
{  
  return 
  {
    gameState->player.pos.x - 4, 
    gameState->player.pos.y - 8, 
    8, 
    16
  };
}

IVec2 get_tile_pos(int x, int y)
{
  return {x * TILESIZE, y * TILESIZE};
}

IRect get_tile_rect(int x, int y)
{
  return {get_tile_pos(x, y), 8, 8};
}

IRect get_solid_rect(Solid solid)
{
  Sprite sprite = get_sprite(solid.spriteID);
  return {solid.pos - sprite.size / 2, sprite.size};
}

void static update_player(float dt, bool enabledFriction, bool enabledGravity, bool enabledGrounded)
{
  // Update Player
  Player& player = gameState->player;
  player.prevPos = player.pos;
  player.animationState = PLAYER_ANIM_IDLE;

  static Vec2 remainder = {};
  static bool grounded = enabledGrounded;
  constexpr float runSpeed = 1.5f;
  constexpr float runAcceleration = 10.0f;
  constexpr float runReduce = 22.0f; 
  constexpr float flyReduce = 12.0f;    
  constexpr float gravity = 13.0f;
  constexpr float fallSpeed = 3.6f;
  constexpr float jumpSpeed = -3.0f;

  // Facing the Player in the right direction
  if(player.speed.x > 0)
  {
    player.renderOptions = 0;
  }
  if(player.speed.x < 0)
  {
    player.renderOptions = RENDER_OPTION_FLIP_X;
  }

  // Jump
  if(just_pressed(JUMP) && grounded)
  {
    player.speed.y = jumpSpeed;
    player.speed.x += player.solidSpeed.x;
    player.speed.y += player.solidSpeed.y;
    play_sound("jump");
    grounded = false;
  }

  if(!grounded)
  {
    player.animationState = PLAYER_ANIM_JUMP;
  }

  if(is_down(MOVE_LEFT))
  {
    if(grounded)
    {
      player.animationState = PLAYER_ANIM_RUN;
    }

    float mult = 1.0f;
    if(player.speed.x > 0.0f)
    {
      mult = 3.0f;
    }
    player.runAnimTime += dt;
    player.speed.x = approach(player.speed.x, -runSpeed, runAcceleration * mult * dt);
  }

  if(is_down(MOVE_RIGHT))
  {
    if(grounded)
    {
      player.animationState = PLAYER_ANIM_RUN;
    }

    float mult = 1.0f;
    if(player.speed.x < 0.0f)
    {
      mult = 3.0f;
    }
    player.runAnimTime += dt;
    player.speed.x = approach(player.speed.x, runSpeed, runAcceleration * mult * dt);
  }

  if(is_down(MOVE_UP))
  {
    if(grounded)
    {
      player.animationState = PLAYER_ANIM_RUN;
    }

    float mult = 1.0f;
    if(player.speed.y < 0.0f)
    {
      mult = 3.0f;
    }
    player.runAnimTime += dt;
    player.speed.y = approach(player.speed.y, -runSpeed, runAcceleration * mult * dt);
  }

  if(is_down(MOVE_DOWN))
  {
    if(grounded)
    {
      player.animationState = PLAYER_ANIM_RUN;
    }

    float mult = 1.0f;
    if(player.speed.y < 0.0f)
    {
      mult = 3.0f;
    }
    player.runAnimTime += dt;
    player.speed.y = approach(player.speed.y, runSpeed, runAcceleration * mult * dt);
  }

  // Friction
  if (enabledFriction)
  {
    if(!is_down(MOVE_LEFT) &&
      !is_down(MOVE_RIGHT))
    {
      if(grounded)
      {
        player.speed.x = approach(player.speed.x, 0, runReduce * dt);
      }
      else
      {
        player.speed.x = approach(player.speed.x, 0, flyReduce * dt);
      }
    }

    if(!is_down(MOVE_UP) &&
      !is_down(MOVE_DOWN))
    {
      // Add Friction to Y axis for non-platformers
      player.speed.y = approach(player.speed.y, 0, runReduce * dt);
    }
  }

  // Gravity
  if (enabledGravity)
  {
    player.speed.y = approach(player.speed.y, fallSpeed, gravity * dt);


    if(is_down(MOVE_UP))
    {
      player.pos = {};
    }
  }

  // Move X
  {
    IRect playerRect = get_player_rect();

    remainder.x += player.speed.x;
    int moveX = round(remainder.x);
    if(moveX != 0)
    {
      remainder.x -= moveX;
      int moveSign = sign(moveX);
      bool collisionHappened = false;

      // Move the player in Y until collision or moveY is exausted
      auto movePlayerX = [&]
      {
        while(moveX)
        {
          playerRect.pos.x += moveSign;

          // Update camera to follow player
          renderData->gameCamera.position.x = player.pos.x;

          // Test collision against Solids
          {
            for(int solidIdx = 0; solidIdx < gameState->solidsLevel1.count; solidIdx++)
            {
              Solid& solid = gameState->solidsLevel1[solidIdx];
              IRect solidRect = get_solid_rect(solid);

              if(rect_collision(playerRect, solidRect))
              {
                player.speed.x = 0;
                return;
              }
            }
          }

          // Loop through local Tiles
          IVec2 playerGridPos = get_grid_pos(player.pos);
          for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
          {
            for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
            {
              Tile* tile = get_tile(x, y);

              if(!tile || !tile->isVisible)
              {
                continue;
              }

              IRect tileRect = get_tile_rect(x, y);
              if(rect_collision(playerRect, tileRect))
              {
                player.speed.x = 0;
                return;
              }
            }
          }

          // Move the Player
          player.pos.x += moveSign;
          moveX -= moveSign;
        }
      };
      movePlayerX();
    }
  }

  // Move Y
  {
    IRect playerRect = get_player_rect();

    remainder.y += player.speed.y;
    int moveY = round(remainder.y);
    if(moveY != 0)
    {
      remainder.y -= moveY;
      int moveSign = sign(moveY);
      bool collisionHappened = false;

      // Move the player in Y until collision or moveY is exausted
      auto movePlayerY = [&]
      {
        while(moveY)
        {
          playerRect.pos.y += moveSign;

          // Update camera to follow player
          renderData->gameCamera.position.y = -player.pos.y;

          // Test collision against Solids
          {
            for(int solidIdx = 0; solidIdx < gameState->solidsLevel1.count; solidIdx++)
            {
              Solid& solid = gameState->solidsLevel1[solidIdx];
              IRect solidRect = get_solid_rect(solid);

              if(rect_collision(playerRect, solidRect))
              {
                // Moving down/falling
                /* if(player.speed.y > 0.0f)
                {
                  grounded = true;
                } */

                player.speed.y = 0;
                return;
              }
            }
          }

          // Loop through local Tiles
          IVec2 playerGridPos = get_grid_pos(player.pos);
          for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
          {
            for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
            {
              Tile* tile = get_tile(x, y);

              if(!tile || !tile->isVisible)
              {
                continue;
              }

              IRect tileRect = get_tile_rect(x, y);
              if(rect_collision(playerRect, tileRect))
              {
                // Moving down/falling
                /* if(player.speed.y > 0.0f)
                {
                  grounded = true;
                } */

                player.speed.y = 0;
                return;
              }
            }
          }

          // Move the Player
          player.pos.y += moveSign;
          moveY -= moveSign;
        }
      };
      movePlayerY();
    }
  }
}

void static update_solids_level_1(float dt)
{
  Player& player = gameState->player;
  player.solidSpeed = {};

  for(int solidIdx = 0; solidIdx < gameState->solidsLevel1.count; solidIdx++)
  {
    Solid& solid = gameState->solidsLevel1[solidIdx];
    solid.prevPos = solid.pos;
    solid.keyframes[0] = gameState->player.pos;

    IRect solidRect = get_solid_rect(solid);
    solidRect.pos -= 1;
    solidRect.size += 2;

    int nextKeyframeIdx = solid.keyframeIdx + 1;
    nextKeyframeIdx %= solid.keyframes.count;

    // Move X
    {
      solid.remainder.x += solid.speed.x * dt;
      int moveX = round(solid.remainder.x);
      if(moveX != 0)
      {
        solid.remainder.x -= moveX;
        int moveSign = sign(solid.keyframes[nextKeyframeIdx].x - 
                            solid.keyframes[solid.keyframeIdx].x);

        // Move the player in Y until collision or moveY is exausted
        auto moveSolidX = [&]
        {
          while(moveX)
          {
            IRect playerRect = get_player_rect();
            bool standingOnTop = 
              playerRect.pos.y - 1 + playerRect.size.y == solidRect.pos.y;

            solidRect.pos.x += moveSign;

            // Collision happend on left or right, push the player
            bool tileCollision = false;
            if(rect_collision(playerRect, solidRect))
            {
              // Move the player rect
              playerRect.pos.x += moveSign;
              player.solidSpeed.x = solid.speed.x * (float)moveSign / 20.0f;

              // Check for collision, if yes, destroy the player
              // Loop through local Tiles
              IVec2 playerGridPos = get_grid_pos(player.pos);
              for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
              {
                for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                {
                  Tile* tile = get_tile(x, y);

                  if(!tile || !tile->isVisible)
                  {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if(rect_collision(playerRect, tileRect))
                  {
                    tileCollision = true;

                    if(!standingOnTop)
                    {
                      // Death
                    player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                    }
                  }
                }
              }

              if(!tileCollision)
              {
                // Actually move the player
                player.pos.x += moveSign;
              }
            }

            // Move the Solid
            solid.pos.x += moveSign;
            moveX -= 1;

            if(solid.pos.x == solid.keyframes[nextKeyframeIdx].x)
            {
              solid.keyframeIdx = nextKeyframeIdx;
              nextKeyframeIdx++;
              nextKeyframeIdx %= solid.keyframes.count;
            }
          }
        };
        moveSolidX();
      }
    }

    // Move Y
    {
      solid.remainder.y += solid.speed.y * dt;
      int moveY = round(solid.remainder.y);
      if(moveY != 0)
      {
        solid.remainder.y -= moveY;
        int moveSign = sign(solid.keyframes[nextKeyframeIdx].y - 
                            solid.keyframes[solid.keyframeIdx].y);

        // Move the player in Y until collision or moveY is exausted
        auto moveSolidY = [&]
        {
          while(moveY)
          {
            IRect playerRect = get_player_rect();
            solidRect.pos.x += moveSign;

            // Collision happend on bottom, push the player
            if(rect_collision(playerRect, solidRect))
            {
              // Move the player
              player.pos.y += moveSign;
              player.solidSpeed.y = solid.speed.y * (float)moveSign / 40.0f;

              // Check for collision, if yes, destroy the player
              // Loop through local Tiles
              IVec2 playerGridPos = get_grid_pos(player.pos);
              for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
              {
                for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                {
                  Tile* tile = get_tile(x, y);

                  if(!tile || !tile->isVisible)
                  {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if(rect_collision(playerRect, tileRect))
                  {
                    player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                  }
                }
              }
            }

            // Move the Solid
            solid.pos.y += moveSign;
            moveY -= 1;

            if(solid.pos.y == solid.keyframes[nextKeyframeIdx].y)
            {
              solid.keyframeIdx = nextKeyframeIdx;
              nextKeyframeIdx++;
              nextKeyframeIdx %= solid.keyframes.count;
            }
          }
        };
        moveSolidY();
      }
    }
  }
}

void static update_solids_level_2(float dt)
{
  Player& player = gameState->player;
  player.solidSpeed = {};

  for(int solidIdx = 0; solidIdx < gameState->solidsLevel2.count; solidIdx++)
  {
    Solid& solid = gameState->solidsLevel2[solidIdx];
    solid.prevPos = solid.pos;

    IRect solidRect = get_solid_rect(solid);
    solidRect.pos -= 1;
    solidRect.size += 2;

    int nextKeyframeIdx = solid.keyframeIdx + 1;
    nextKeyframeIdx %= solid.keyframes.count;

    // Move X
    {
      solid.remainder.x += solid.speed.x * dt;
      int moveX = round(solid.remainder.x);
      if(moveX != 0)
      {
        solid.remainder.x -= moveX;
        int moveSign = sign(solid.keyframes[nextKeyframeIdx].x - 
                            solid.keyframes[solid.keyframeIdx].x);

        // Move the player in Y until collision or moveY is exausted
        auto moveSolidX = [&]
        {
          while(moveX)
          {
            IRect playerRect = get_player_rect();
            bool standingOnTop = 
              playerRect.pos.y - 1 + playerRect.size.y == solidRect.pos.y;

            solidRect.pos.x += moveSign;

            // Collision happend on left or right, push the player
            bool tileCollision = false;
            if(rect_collision(playerRect, solidRect))
            {
              // Move the player rect
              playerRect.pos.x += moveSign;
              player.solidSpeed.x = solid.speed.x * (float)moveSign / 20.0f;

              // Check for collision, if yes, destroy the player
              // Loop through local Tiles
              IVec2 playerGridPos = get_grid_pos(player.pos);
              for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
              {
                for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                {
                  Tile* tile = get_tile(x, y);

                  if(!tile || !tile->isVisible)
                  {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if(rect_collision(playerRect, tileRect))
                  {
                    tileCollision = true;

                    if(!standingOnTop)
                    {
                      // Death
                    player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                    }
                  }
                }
              }

              if(!tileCollision)
              {
                // Actually move the player
                player.pos.x += moveSign;
              }
            }

            // Move the Solid
            solid.pos.x += moveSign;
            moveX -= 1;

            if(solid.pos.x == solid.keyframes[nextKeyframeIdx].x)
            {
              solid.keyframeIdx = nextKeyframeIdx;
              nextKeyframeIdx++;
              nextKeyframeIdx %= solid.keyframes.count;
            }
          }
        };
        moveSolidX();
      }
    }

    // Move Y
    {
      solid.remainder.y += solid.speed.y * dt;
      int moveY = round(solid.remainder.y);
      if(moveY != 0)
      {
        solid.remainder.y -= moveY;
        int moveSign = sign(solid.keyframes[nextKeyframeIdx].y - 
                            solid.keyframes[solid.keyframeIdx].y);

        // Move the player in Y until collision or moveY is exausted
        auto moveSolidY = [&]
        {
          while(moveY)
          {
            IRect playerRect = get_player_rect();
            solidRect.pos.x += moveSign;

            // Collision happend on bottom, push the player
            if(rect_collision(playerRect, solidRect))
            {
              // Move the player
              player.pos.y += moveSign;
              player.solidSpeed.y = solid.speed.y * (float)moveSign / 40.0f;

              // Check for collision, if yes, destroy the player
              // Loop through local Tiles
              IVec2 playerGridPos = get_grid_pos(player.pos);
              for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
              {
                for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                {
                  Tile* tile = get_tile(x, y);

                  if(!tile || !tile->isVisible)
                  {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if(rect_collision(playerRect, tileRect))
                  {
                    player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                  }
                }
              }
            }

            // Move the Solid
            solid.pos.y += moveSign;
            moveY -= 1;

            if(solid.pos.y == solid.keyframes[nextKeyframeIdx].y)
            {
              solid.keyframeIdx = nextKeyframeIdx;
              nextKeyframeIdx++;
              nextKeyframeIdx %= solid.keyframes.count;
            }
          }
        };
        moveSolidY();
      }
    }
  }
}

void static update_solids(float dt)
{
  if (gameState->state == GAME_STATE_IN_LEVEL_1)
  {
    update_solids_level_1(dt);
  }
  else if (gameState->state == GAME_STATE_IN_LEVEL_2)
  {
    update_solids_level_2(dt);
  }
}

void static update_enemies(float dt)
{
  if (gameState->state == GAME_STATE_IN_LEVEL_1)
  {
    for (int i = 0; i < gameState->enemiesLevel1.count; i++)
    {
      gameState->enemiesLevel1[i].keyframes[0] = gameState->player.pos;
    }
  }
}

void update_level(float dt)
{
  if(just_pressed(PAUSE))
  {
    gameState->state = GAME_STATE_MAIN_MENU;
  }

  // Update Background
  {
    // Calculate the player's current tile
    Player& player = gameState->player;

    // Get tile cell that the player is in
    IVec2 playerTile = get_grid_pos(IVec2{player.pos.x, player.pos.y});

    // @TODO not efficient
    gameState->backgroundTiles.clear();

    // Define X of starting tile position
    // (get starting tile position) - (offset to take in account for dynamic tile grid sizes)
    int startingXPos = (playerTile.x * TILESIZE) - (TILESIZE * (NUM_OF_TILE_COLUMNS / 2));

    // Dynamically generate tiles for background based on # of columns and rows
    for (int column = 0; column < NUM_OF_TILE_COLUMNS; column++)
    {
      // Define Y of starting tile position
      int startingYPos = (playerTile.y * TILESIZE) + (TILESIZE * (NUM_OF_TILE_ROWS / 2));

      for (int row = 0; row < NUM_OF_TILE_ROWS; row++)
      {
        IRect tile = {};
        tile.pos = IVec2{startingXPos, startingYPos};
        tile.size = IVec2{TILESIZE, TILESIZE};
        gameState->backgroundTiles.add(tile);

        // Modify Y for next row entry
        startingYPos -= TILESIZE;
      }

      // Modify X for next column entry
      startingXPos += TILESIZE;
    }
  }

  // Updates for the main player
  update_player(dt, true, false, true);

  // Update camera to follow player
  Player& player = gameState->player;
  renderData->gameCamera.position.x = player.pos.x;
  renderData->gameCamera.position.y = -player.pos.y;

  // Update Solids
  update_solids(dt);

  // Update Enemies
  //update_enemies(dt);

  bool updateTiles = false;
  /*if(is_down(MOUSE_LEFT) && !ui_is_hot() && !ui_is_active())
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if(tile)
    {
      tile->isVisible = true;
      updateTiles = true;
    }
  }

  if(is_down(MOUSE_RIGHT))
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if(tile)
    {
      tile->isVisible = false;
      updateTiles = true;
    }
  }*/

  /*if(updateTiles)
  {
    // Neighbouring Tiles        Top    Left      Right       Bottom  
    int neighbourOffsets[24] = { 0,-1,  -1, 0,     1, 0,       0, 1,   
    //                          Topleft Topright Bottomleft Bottomright
                                -1,-1,   1,-1,    -1, 1,       1, 1,
    //                           Top2   Left2     Right2      Bottom2
                                 0,-2,  -2, 0,     2, 0,       0, 2};

    // Topleft     = BIT(4) = 16
    // Toplright   = BIT(5) = 32
    // Bottomleft  = BIT(6) = 64
    // Bottomright = BIT(7) = 128

    for(int y = 0; y < WORLD_GRID.y; y++)
    {
      for(int x = 0; x < WORLD_GRID.x; x++)
      {
        Tile* tile = get_tile(x, y);

        if(!tile->isVisible)
        {
          continue;
        }

        tile->neighbourMask = 0;
        int neighbourCount = 0;
        int extendedNeighbourCount = 0;
        int emptyNeighbourSlot = 0;

        // Look at the sorrounding 12 Neighbours
        for(int n = 0; n < 12; n++)
        {
          Tile* neighbour = get_tile(x + neighbourOffsets[n * 2],
                                     y + neighbourOffsets[n * 2 + 1]);

          // No neighbour means the edge of the world
          if(!neighbour || neighbour->isVisible)
          {
            tile->neighbourMask |= BIT(n);
            if(n < 8) // Counting direct neighbours
            {
              neighbourCount++;
            }
            else // Counting neighbours 1 Tile away
            {
              extendedNeighbourCount++;
            }
          }
          else if(n < 8)
          {
            emptyNeighbourSlot = n;
          }
        }

        if(neighbourCount == 7 && emptyNeighbourSlot >= 4) // We have a corner
        {
          tile->neighbourMask = 16 + (emptyNeighbourSlot - 4);
        }
        else if(neighbourCount == 8 && extendedNeighbourCount == 4)
        {
          tile->neighbourMask = 20;
        }
        else
        {
          tile->neighbourMask = tile->neighbourMask & 0b1111;
        }
      }
    }
  }*/
}

void update_main_menu(float dt)
{
  int button1_ID = line_id(1);
  int button2_ID = line_id(2);
  Vec4 color = COLOR_WHITE;

  if(is_hot(button1_ID))
  {
    color = COLOR_GREEN;
  }

  // Original button
  if(do_button(SPRITE_BUTTON_PLAY, IVec2{WORLD_WIDTH/2, WORLD_HEIGHT/2}, 
               button1_ID, {.material{.color = color}, .layer = get_layer(LAYER_UI, 10.0f)}))
  {
    gameState->state = GAME_STATE_IN_LEVEL_1;
  }

  // Reset color
  color = COLOR_WHITE;

  if(is_hot(button2_ID))
  {
    color = COLOR_RED;
  }

  // New button
  if(do_button(SPRITE_BUTTON_PLAY, IVec2{WORLD_WIDTH/2, (WORLD_HEIGHT * 3)/4}, 
               button2_ID, {.material{.color = color}, .layer = get_layer(LAYER_UI, 10.0f)}))
  {
    gameState->state = GAME_STATE_IN_LEVEL_2;
  }

  do_ui_text(_(STRING_CELESTE_CLONE), {56, 20}, 
             {.material{.color = COLOR_BLACK}, 
             .fontSize = 2.0f, 
             .layer = get_layer(LAYER_UI, 10)});

  // Fullscreen White quad for the menu
  do_ui_quad(
    {(float)WORLD_WIDTH / 2, (float)WORLD_HEIGHT / 2},
    {(float)WORLD_WIDTH, (float)WORLD_HEIGHT},
    {      
      .material{.color = {79.0f / 255.0f, 140.0f / 255.0f, 235.0f / 255.0f, 1.0f}},
      .layer = get_layer(LAYER_UI, 0.0f)
    });
}

void simulate()
{
  float dt = UPDATE_DELAY;

  switch(gameState->state)
  {
    case GAME_STATE_IN_LEVEL_1:
    {
      update_level(dt);
      break;
    }
  
    case GAME_STATE_IN_LEVEL_2:
    {
      update_level(dt);
      break;
    }

    case GAME_STATE_MAIN_MENU:
    {
      update_main_menu(dt);
      break;
    }
  }
}

// #############################################################################
//                           Game Functions(exposed)
// #############################################################################
EXPORT_FN void update_game(GameState* gameStateIn, 
                           RenderData* renderDataIn, 
                           Input* inputIn, 
                           SoundState* soundStateIn,
                           UIState* uiStateIn,
                           float dt)
{
  if(renderData != renderDataIn)
  {
    gameState = gameStateIn;
    renderData = renderDataIn;
    input = inputIn;
    soundState = soundStateIn;
    uiState = uiStateIn;

    init_strings();
  }

  if(!gameState->initialized)
  {
    // play_sound("First Steps", SOUND_OPTION_LOOP);
    renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
    renderData->gameCamera.position.x = 160;
    renderData->gameCamera.position.y = -90;

    renderData->uiCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
    renderData->uiCamera.position.x = 160;
    renderData->uiCamera.position.y = -90;

    // Player
    {
      Player& player = gameState->player;
      player.animationSprites[PLAYER_ANIM_IDLE] = SPRITE_CELESTE;
      player.animationSprites[PLAYER_ANIM_JUMP] = SPRITE_CELESTE_JUMP;
      player.animationSprites[PLAYER_ANIM_RUN] = SPRITE_CELESTE_RUN;
    }

    // Key Mappings
    {
      gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
      gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);
      //gameState->keyMappings[MOUSE_LEFT].keys.add(KEY_MOUSE_LEFT);
      //gameState->keyMappings[MOUSE_RIGHT].keys.add(KEY_MOUSE_RIGHT);
      gameState->keyMappings[JUMP].keys.add(KEY_SPACE);
      gameState->keyMappings[PAUSE].keys.add(KEY_ESCAPE);
    }

    // Solids
    {
      Solid solid = {};
      solid.spriteID = SPRITE_SOLID_01;
      solid.keyframes.add({8 * 2,  8 * 10});
      solid.keyframes.add({8 * 10, 8 * 10});
      solid.pos = {8 * 2, 8 * 10};
      solid.speed.x = 50.0f;
      gameState->solidsLevel1.add(solid);

      solid = {};
      solid.spriteID = SPRITE_SOLID_02;
      solid.keyframes.add({12 * 20, 8 * 10});
      solid.keyframes.add({12 * 20, 8 * 20});
      solid.pos = {12 * 20, 8 * 10};
      solid.speed.y = 50.0f;
      gameState->solidsLevel1.add(solid);
    }

    // Enemies
    {
      /*Solid enemy = {};
      enemy.spriteID = SPRITE_SOLID_02;
      enemy.keyframes.add(gameState->player.pos);
      //enemy.keyframes.add({8 * 10, 8 * 10});
      enemy.pos = {8 * 2, 8 * 10};
      enemy.speed.x = 50.0f;
      gameState->enemiesLevel1.add(enemy);

      enemy = {};
      enemy.spriteID = SPRITE_SOLID_02;
      enemy.keyframes.add(gameState->player.pos);
      //enemy.keyframes.add({12 * 20, 8 * 20});
      enemy.pos = {12 * 20, 8 * 10};
      enemy.speed.y = 50.0f;
      gameState->enemiesLevel1.add(enemy);*/
    }

    gameState->initialized = true;
  }

  // Fixed Update Loop
  {
    gameState->updateTimer += dt;
    while(gameState->updateTimer >= UPDATE_DELAY)
    {
      gameState->updateTimer -= UPDATE_DELAY;
      update_ui();
      simulate();

      // Relative Mouse here, because more frames than simulations
      input->relMouse = input->mousePos - input->prevMousePos;
      input->prevMousePos = input->mousePos;

      // Clear the transitionCount for every key
      {
        for (int keyCode = 0; keyCode < KEY_COUNT; keyCode++)
        {
          input->keys[keyCode].justReleased = false;
          input->keys[keyCode].justPressed = false;
          input->keys[keyCode].halfTransitionCount = 0;
        }
      }
    }
  }

  float interpolatedDT = (float)(gameState->updateTimer / UPDATE_DELAY);
  
  // *GENERAL NOTE* Order of drawing matters here, background should be first

  // Draw background tiles
  {
    DrawData tileData;
    tileData.layer = get_layer(LAYER_GAME, 0);

    // Draw all background tiles
    for (int i = 0; i < gameState->backgroundTiles.count; i++)
    {
      draw_sprite(SPRITE_TILE_GRASS_01, gameState->backgroundTiles[i].pos, tileData);
    }
  }

  // Draw UI
  {
    for(int uiElementIdx = 0; uiElementIdx < uiState->uiElements.count; uiElementIdx++)
    {
      UIElement& uiElement = uiState->uiElements[uiElementIdx];
      draw_ui_sprite(uiElement.spriteID, uiElement.pos, uiElement.size, uiElement.drawData);
    }

    for(int uiTextIdx = 0; uiTextIdx < uiState->uiTexts.count; uiTextIdx++)
    {
      UIText& uiText = uiState->uiTexts[uiTextIdx];
      draw_ui_text(uiText.text, uiText.pos, uiText.textData);
    }
  }

  // Draw Solids
  {
    for(int solidIdx = 0; solidIdx < gameState->solidsLevel1.count; solidIdx++)
    {
      Solid& solid = gameState->solidsLevel1[solidIdx];
      IVec2 solidPos = lerp(solid.prevPos, solid.pos, interpolatedDT);
      draw_sprite(solid.spriteID, solidPos, {.layer = get_layer(LAYER_GAME, 1)});
    }
  }

  // Draw Enemies
  /*for(int enemyIdx = 0; enemyIdx < gameState->enemiesLevel1.count; enemyIdx++)
    {
      Solid& enemy = gameState->enemiesLevel1[enemyIdx];
      IVec2 enemyPos = lerp(enemy.prevPos, enemy.pos, interpolatedDT);
      draw_sprite(enemy.spriteID, enemyPos, {.layer = get_layer(LAYER_GAME, 2)});
    }*/

  // Draw Player
  {
    Player& player = gameState->player;
    IVec2 playerPos = lerp(player.prevPos, player.pos, interpolatedDT);
    
    Sprite sprite = get_sprite(player.animationSprites[player.animationState]);
    int animationIdx = animate(&player.runAnimTime, sprite.frameCount, 0.6f);
    draw_sprite(player.animationSprites[player.animationState], playerPos, 
                {
                  .animationIdx = animationIdx,
                  .renderOptions = player.renderOptions,
                  .layer = get_layer(LAYER_GAME, 2)
                });
  }
}