#include "Progression.h"

void InitProgression(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

void ProgressionSetOpening(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

void ProgressionSetTutorial(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_TUTORIAL;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

void ProgressionSetLevel(GameProgression* progression, int levelIndex, bool combatEnabled)
{
    if (!progression) return;

    progression->chapter = CHAPTER_LEVEL;
    progression->levelIndex = levelIndex;
    progression->combatEnabled = combatEnabled;
}
