#include "Progression.h"

// Initialize game flow at opening chapter with combat disabled.
void InitProgression(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

// Explicit setter for starting/restarting opening chapter.
void ProgressionSetOpening(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

// Tutorial chapter is objective-based and non-combat.
void ProgressionSetTutorial(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_TUTORIAL;
    progression->levelIndex = 0;
    progression->combatEnabled = false;
}

// Level chapter supports configurable combat toggle and index.
void ProgressionSetLevel(GameProgression* progression, int levelIndex, bool combatEnabled)
{
    if (!progression) return;

    progression->chapter = CHAPTER_LEVEL;
    progression->levelIndex = levelIndex;
    progression->combatEnabled = combatEnabled;
}
