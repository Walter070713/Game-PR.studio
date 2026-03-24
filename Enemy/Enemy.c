#include "Enemy.h"

// Initialize the enemypool
void InitEnemy(Enemy enemypool[],int emycapacity)
{
    for (int i=0;i<emycapacity;++i)
    {
        enemypool[i].flashtime=0.0f; // the time duration of the quick flash effect when the enemy get hit
        enemypool[i].pos=(Vector2){GetRandomValue(0,window_width),GetRandomValue(0,window_height)};
        enemypool[i].health=3;
        enemypool[i].active=true; // whether it's dead or alive
        enemypool[i].speed=400.0f;
        enemypool[i].body=30.0f; // enemy's size
        enemypool[i].state=WHITE; // the enemy turns red real quick and returns to white when it get hit
        enemypool[i].name=0; // No name by default
    }
}

// Update the enemy horde
void UpdateEnemyHorde(Enemy enemypool[],int emycapacity,Vector2 plpos)
{
    // Loop through the horde so as to update ONLY alive enemies' position 
    for (int i=0;i<emycapacity;++i)
    {
        if (enemypool[i].health==0) // Update dead enemies state
        {
            enemypool[i].active=false;
            enemypool[i].flashtime=0.0f;
        }
        if (enemypool[i].active) // Update alive enemies position
        {
            enemypool[i].prevpos=enemypool[i].pos; // save the current pos
            enemypool[i].pos=Vector2MoveTowards(enemypool[i].pos,plpos,enemypool[i].speed*GetFrameTime());
        }
        if (enemypool[i].flashtime>0) // Check if the enemy is in the RED state and making it return to normal WHITE state looply.
        {
            enemypool[i].flashtime-=GetFrameTime();
        }

    }
}

// Drawing alive enemies
void DrawEnemy(Enemy enemypool[],int emycapacity)
{
    for (int i=0;i<emycapacity;++i)
    {
        // Enemy getting hit effect
        if (enemypool[i].flashtime>0.0f)
        {
            enemypool[i].state=RED;
        }
        else
        {
            enemypool[i].state=WHITE;
        }
        // Drawing alive enemies
        if (enemypool[i].active)
        {
            DrawCircleV(enemypool[i].pos,enemypool[i].body,enemypool[i].state);
            // Draw name if it exists (for NPCs)
            if (enemypool[i].name)
            {
                float fontSize = enemypool[i].body * 1.6f;
                if (fontSize < 10.0f) fontSize = 10.0f;
                if (fontSize > 20.0f) fontSize = 20.0f;
                Vector2 textSize = MeasureTextEx(GetFontDefault(), enemypool[i].name, fontSize, 1.0f);
                Vector2 textPos = {enemypool[i].pos.x - textSize.x * 0.5f, enemypool[i].pos.y - enemypool[i].body - fontSize - 3.0f};
                DrawTextEx(GetFontDefault(), enemypool[i].name, textPos, fontSize, 1.0f, WHITE);
            }
        }
    }
}