#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::string currentBookFile = "unknown_book.txt";

// This will save your highlight to a simple text file
void saveHighlightToFile(std::string text) {
    std::ofstream outfile;
    // 'app' means append - it adds to the end of the file
    outfile.open("highlights.txt", std::ios_base::app); 
    outfile << currentBookFile << " : " << text << "\n";
    outfile.close();
}

void loadHighlights() {
    // Just a placeholder for now to satisfy the compiler
}

void deleteHighlightAt(unsigned long index) {
    // Just a placeholder for now
}

struct Highlight { int dummy; }; 
std::vector<Highlight> getHighlightsForCurrentBook() {
    return std::vector<Highlight>();
}

bool selecting = false;
void startOrFinishHighlight(int dummy) {
    selecting = !selecting;
    if (!selecting) {
        saveHighlightToFile("New Highlight Added");
    }
}
