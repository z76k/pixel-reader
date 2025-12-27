#pragma once
#include <vector>
#include <string>
#include <fstream>

inline bool selecting = false;
inline std::string currentBookFile = "mybook";
struct Highlight { int dummy; };
inline std::vector<Highlight> getHighlightsForCurrentBook() { return {}; }
inline void loadHighlights() {}
inline void deleteHighlightAt(unsigned long i) { (void)i; }
inline void startOrFinishHighlight(int d) { (void)d; selecting = !selecting; }
