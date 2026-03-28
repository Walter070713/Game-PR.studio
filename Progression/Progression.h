#ifndef PROGRESSION_H
#define PROGRESSION_H

#include <stdbool.h>

// Chapter-level progression states used to branch gameplay behavior.
typedef enum {
    CHAPTER_OPENING,
    CHAPTER_TUTORIAL
} ChapterType;

// Minimal progression payload for current chapter.
typedef struct {
    ChapterType chapter;
} GameProgression;

// Initialize progression to starting chapter.
void InitProgression(GameProgression* progression);

// Force progression to opening chapter.
void ProgressionSetOpening(GameProgression* progression);

// Force progression to tutorial chapter.
void ProgressionSetTutorial(GameProgression* progression);

#endif