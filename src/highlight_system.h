#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

// 'inline' lets us include this anywhere without crashing the builder
namespace HighlightSys {

    struct HL { int sL, sC, eL, eC; }; // StartLine, StartChar, EndLine, EndChar
    inline std::vector<HL> db;         // The database of highlights
    inline bool selecting = false;     // Are we dragging right now?
    inline int anchorL = 0, anchorC = 0;

    // --- 1. VISUAL LOGIC: Call this in your draw loop to set color! ---
    inline bool isHighlighted(int line, int col) {
        for(const auto& h : db) {
            // Check if (line, col) is inside the start and end points
            bool afterStart = (line > h.sL) || (line == h.sL && col >= h.sC);
            bool beforeEnd  = (line < h.eL) || (line == h.eL && col < h.eC);
            if (afterStart && beforeEnd) return true;
        }
        return false;
    }

    // --- 2. FILE LOGIC: Saves to "bookname.txt" ---
    inline void save(const std::string& bookName) {
        std::ofstream f(bookName + "_highlights.txt");
        for(const auto& h : db) {
            f << h.sL << " " << h.sC << " " << h.eL << " " << h.eC << "\n";
        }
    }

    inline void load(const std::string& bookName) {
        db.clear();
        std::ifstream f(bookName + "_highlights.txt");
        int a,b,c,d;
        while(f >> a >> b >> c >> d) db.push_back({a,b,c,d});
    }

    // --- 3. ACTION LOGIC: Call this when user presses "Select" ---
    inline void toggleSelection(int l, int c, const std::string& bookName) {
        if (!selecting) {
            selecting = true;
            anchorL = l; anchorC = c;
        } else {
            selecting = false;
            // Ensure Start is always before End
            int sL=anchorL, sC=anchorC, eL=l, eC=c;
            if (sL > eL || (sL == eL && sC > eC)) { 
                std::swap(sL, eL); std::swap(sC, eC); 
            }
            db.push_back({sL, sC, eL, eC});
            save(bookName);
        }
    }
}
