#ifndef PROGRESSION_H
#define PROGRESSION_H

#include <stdbool.h>

typedef enum {
    CHAPTER_OPENING,
    CHAPTER_TUTORIAL,
    CHAPTER_LEVEL
} ChapterType;

typedef struct {
    ChapterType chapter;
    int levelIndex;
    bool combatEnabled;
} GameProgression;

void InitProgression(GameProgression* progression);
void ProgressionSetOpening(GameProgression* progression);
void ProgressionSetTutorial(GameProgression* progression);
void ProgressionSetLevel(GameProgression* progression, int levelIndex, bool combatEnabled);

#endif