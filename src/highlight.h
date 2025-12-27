#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <string>
#include <vector>

extern bool selecting;
extern std::string currentBookFile;

struct HighlightInfo {
    int start;
    int end;
};

void loadHighlights();
void saveHighlight();
void startOrFinishHighlight(int cursorPos);
bool isCharHighlighted(int charIndex);

// Helpers to list/manage highlights for current book
std::vector<HighlightInfo> getHighlightsForCurrentBook();
void deleteHighlightAt(size_t index);

#endif
