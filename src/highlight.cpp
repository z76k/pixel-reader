#include <string>
#include <vector>

std::string currentBookFile = "";

void loadHighlights() {}
void deleteHighlightAt(unsigned long index) {}

struct Highlight { int dummy; }; 
std::vector<Highlight> getHighlightsForCurrentBook() {
    return std::vector<Highlight>();
}

bool selecting = false;
void startOrFinishHighlight(int dummy) {
    selecting = !selecting;
}
