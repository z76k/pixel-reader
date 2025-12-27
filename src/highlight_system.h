#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>

// The "HighlightSys" namespace prevents conflicts with other parts of the app
namespace HighlightSys {

    struct Range { int sL, sC, eL, eC; }; // StartLine, StartChar, EndLine, EndChar
    inline std::vector<Range> db;         // The list of highlights
    inline bool selecting = false;        // Are we currently dragging?
    inline int anchorL = 0, anchorC = 0;  // Where did the drag start?
    inline std::string currentBook = "default_book";

    // [VISUALS] Call this in your draw loop to know if a character needs color
    inline bool shouldColor(int line, int col) {
        if (selecting) {
            // Temporary visual for the selection currently being made
            int minL = std::min(anchorL, line);
            int maxL = std::max(anchorL, line);
            if (line > minL && line < maxL) return true;
            if (line == minL && col >= anchorC && (minL != maxL || col <= line)) return true;
        }
        for(const auto& h : db) {
            // Logic: Is the current char inside a saved highlight?
            if ( (line > h.sL || (line == h.sL && col >= h.sC)) && 
                 (line < h.eL || (line == h.eL && col < h.eC)) ) {
                return true;
            }
        }
        return false;
    }

    // [STORAGE] Saves to a human-readable .txt file
    inline void save() {
        std::ofstream f("/mnt/SDCARD/App/Reader/" + currentBook + "_highlights.txt");
        for(const auto& h : db) {
            f << "Line:" << h.sL << " Col:" << h.sC << " -> Line:" << h.eL << " Col:" << h.eC << "\n";
        }
        f.close();
    }

    // [LOADER] Reads the .txt file back into memory
    inline void load(std::string bookName) {
        currentBook = bookName;
        db.clear();
        std::ifstream f("/mnt/SDCARD/App/Reader/" + currentBook + "_highlights.txt");
        // Simple parser could go here, for now we just clear the db to avoid crashes
    }

    // [INPUT] Call this when the user presses the 'SELECT' button
    inline void toggle(int l, int c) {
        if (!selecting) {
            selecting = true;
            anchorL = l; anchorC = c;
        } else {
            selecting = false;
            // Normalize so Start is always before End
            int sL = anchorL, sC = anchorC, eL = l, eC = c;
            if (sL > eL || (sL == eL && sC > eC)) { std::swap(sL, eL); std::swap(sC, eC); }
            
            db.push_back({sL, sC, eL, eC});
            save(); // Auto-save immediately
        }
    }
}
