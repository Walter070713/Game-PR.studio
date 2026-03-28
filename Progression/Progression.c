#include "Progression.h"

// Start progression at opening chapter.
void InitProgression(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
}

// Switch progression back to opening chapter.
void ProgressionSetOpening(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_OPENING;
}

// Advance progression to tutorial chapter.
void ProgressionSetTutorial(GameProgression* progression)
{
    if (!progression) return;

    progression->chapter = CHAPTER_TUTORIAL;
}
