bool selecting = false;
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <SDL/SDL.h>

// ---------- Highlight Data ----------
struct Highlight {
    int start;
    int end;
};

std::vector<Highlight> highlights;
bool selecting = false;
int selStart = 0;
int selEnd = 0;

// File path for saving highlights (inside Books folder)
std::string highlightFilePath = "Books/highlights.txt";

// Name of the current book file (update dynamically in main reader)
std::string currentBookFile = "current_book.txt";

// ---------- Load Highlights ----------
void loadHighlights() {
    highlights.clear();
    std::ifstream file(highlightFilePath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string book;
        int start, end;
        if (iss >> book >> start >> end) {
            if (book == currentBookFile) {
                highlights.push_back({start, end});
            }
        }
    }
    file.close();
}

// ---------- Save a New Highlight ----------
void saveHighlight() {
    if (selEnd < selStart) std::swap(selStart, selEnd);
    highlights.push_back({selStart, selEnd});

    std::ofstream file(highlightFilePath, std::ios::app);
    if (file.is_open()) {
        file << currentBookFile << " " << selStart << " " << selEnd << "\n";
        file.close();
    }
    selecting = false;
}

// ---------- Start or Finish Highlight ----------
void startOrFinishHighlight(int cursorPos) {
    if (!selecting) {
        selecting = true;
        selStart = cursorPos;
        selEnd = cursorPos;
    } else {
        selEnd = cursorPos;
        saveHighlight();
    }
}

// ---------- Check if Character is Highlighted ----------
bool isCharHighlighted(int charIndex) {
    for (auto &h : highlights) {
        if (charIndex >= h.start && charIndex <= h.end) return true;
    }
    return false;
}

// ---------- New helpers ----------
std::vector<Highlight> getHighlightsForCurrentBook()
{
    std::vector<Highlight> out;
    for (const auto &h : highlights) {
        out.push_back({h.start, h.end});
    }
    return out;
}

void deleteHighlightAt(size_t index)
{
    if (index >= highlights.size()) return;

    // Identify the highlight to remove
    Highlight target = highlights[index];

    // Read all existing lines
    std::ifstream infile(highlightFilePath);
    std::vector<std::tuple<std::string,int,int>> lines;
    if (infile.is_open()) {
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string book;
            int s, e;
            if (iss >> book >> s >> e) {
                lines.emplace_back(book, s, e);
            }
        }
        infile.close();
    }

    // Remove the first line that matches the target for current book
    bool removed = false;
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        auto [book, s, e] = *it;
        if (!removed && book == currentBookFile && s == target.start && e == target.end) {
            lines.erase(it);
            removed = true;
            break;
        }
    }

    // Rewrite file with remaining lines
    std::ofstream outfile(highlightFilePath, std::ios::trunc);
    if (!outfile.is_open()) return;
    for (const auto &t : lines) {
        outfile << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << "\n";
    }
    outfile.close();

    // Reload into memory
    loadHighlights();
}
