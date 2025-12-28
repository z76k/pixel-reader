#ifndef HIGHLIGHT_MANAGER_H
#define HIGHLIGHT_MANAGER_H

#include <string>
#include <vector>
#include <ctime>
struct Highlight {
    std::string book_title;
    std::string text;           // The actual highlighted text
    std::string note;           // Optional note
    int start_pos;              // Character position (for jumping back)
    int end_pos;
    std::string date;           // "December 2024"
    
    // Constructor
    Highlight(const std::string& title, const std::string& txt, 
              int start, int end, const std::string& nt = "")
        : book_title(title), text(txt), start_pos(start), 
          end_pos(end), note(nt) {
        // Auto-generate date as "Month Year"
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        char buf[32];
        strftime(buf, sizeof(buf), "%B %Y", t);
        date = buf;
    }
};

class HighlightManager {
public:
    HighlightManager();
    ~HighlightManager();
    
    // Save a new highlight
    bool save_highlight(const Highlight& h);
    
    // Load all highlights for a specific book
    std::vector<Highlight> load_highlights(const std::string& book_title);
    
    // Load ALL highlights (for viewing all)
    std::vector<Highlight> load_all_highlights();
    
    // Delete a specific highlight
    bool delete_highlight(int index);
    
    // Get highlights file path
    std::string get_highlights_path() const;
    
private:
    std::string highlights_file;
    
    // Helper: Parse a line from the file
    Highlight parse_line(const std::string& line);
    
    // Helper: Convert highlight to file format
    std::string to_file_format(const Highlight& h);
};

#endif // HIGHLIGHT_MANAGER_H

