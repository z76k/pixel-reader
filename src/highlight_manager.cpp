#include "highlight_manager.h"
#include <fstream>
#include <sstream>
#include <ctime>

HighlightManager::HighlightManager() {
    // Save highlights in same directory as pixel-reader
    highlights_file = "./highlights.txt";
}

HighlightManager::~HighlightManager() {}

std::string HighlightManager::get_highlights_path() const {
    return highlights_file;
}

bool HighlightManager::save_highlight(const Highlight& h) {
    std::ofstream file(highlights_file, std::ios::app);
    if (!file.is_open()) return false;
    
    file << to_file_format(h) << "\n";
    file.close();
    return true;
}

std::vector<Highlight> HighlightManager::load_highlights(const std::string& book_title) {
    std::vector<Highlight> results;
    std::ifstream file(highlights_file);
    if (!file.is_open()) return results;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        Highlight h = parse_line(line);
        if (h.book_title == book_title) {
            results.push_back(h);
        }
    }
    file.close();
    return results;
}

std::vector<Highlight> HighlightManager::load_all_highlights() {
    std::vector<Highlight> results;
    std::ifstream file(highlights_file);
    if (!file.is_open()) return results;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        try {
            results.push_back(parse_line(line));
        } catch (...) {
            // Skip corrupted lines
            continue;
        }
    }
    file.close();
    return results;
}

bool HighlightManager::delete_highlight(int index) {
    auto all = load_all_highlights();
    if (index < 0 || index >= static_cast<int>(all.size())) return false;
    
    // Remove the highlight at index
    all.erase(all.begin() + index);
    
    // Rewrite file
    std::ofstream file(highlights_file, std::ios::trunc);
    if (!file.is_open()) return false;
    
    for (const auto& h : all) {
        file << to_file_format(h) << "\n";
    }
    file.close();
    return true;
}

Highlight HighlightManager::parse_line(const std::string& line) {
    std::stringstream ss(line);
    std::string book, text, note, pos, date;
    
    std::getline(ss, book, '|');
    std::getline(ss, text, '|');
    std::getline(ss, note, '|');
    std::getline(ss, pos, '|');
    std::getline(ss, date, '|');
    
    // Parse position "1000-1050"
    int start = 0, end = 0;
    size_t dash = pos.find('-');
    if (dash != std::string::npos) {
        start = std::stoi(pos.substr(0, dash));
        end = std::stoi(pos.substr(dash + 1));
    }
    
    Highlight h(book, text, start, end, note);
    h.date = date;
    return h;
}

std::string HighlightManager::to_file_format(const Highlight& h) {
    std::stringstream ss;
    ss << h.book_title << "|"
       << h.text << "|"
       << h.note << "|"
       << h.start_pos << "-" << h.end_pos << "|"
       << h.date;
    return ss.str();
}

