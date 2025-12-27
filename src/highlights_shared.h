#pragma once
#include <string>
#include <vector>

// Using 'inline' allows these to be shared across the whole app
inline bool selecting = false;
inline std::string currentBookFile = "";
inline void loadHighlights() {}
inline void deleteHighlightAt(unsigned long index) { (void)index; }
inline void startOrFinishHighlight(int dummy) { (void)dummy; selecting = !selecting; }

struct Highlight { int dummy; };
inline std::vector<Highlight> getHighlightsForCurrentBook() { return {}; }
