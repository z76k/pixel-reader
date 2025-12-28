#include "src/highlight_manager.h"
#include <iostream>

int main() {
    std::cout << "=== Testing Highlight Manager ===" << std::endl;
    
    HighlightManager mgr;
    
    // Test 1: Create and save a highlight
    Highlight h1("Moby Dick", "Call me Ishmael", 1000, 1015, "Great opening!");
    if (mgr.save_highlight(h1)) {
        std::cout << "✓ Saved highlight 1" << std::endl;
    } else {
        std::cout << "✗ Failed to save highlight 1" << std::endl;
    }
    
    // Test 2: Save another highlight
    Highlight h2("1984", "Big Brother is watching", 5000, 5023, "Creepy");
    mgr.save_highlight(h2);
    std::cout << "✓ Saved highlight 2" << std::endl;
    
    // Test 3: Load all highlights
    auto all = mgr.load_all_highlights();
    std::cout << "\n=== All Highlights ===" << std::endl;
    for (size_t i = 0; i < all.size(); i++) {
        std::cout << i+1 << ". " << all[i].book_title << std::endl;
        std::cout << "   \"" << all[i].text << "\"" << std::endl;
        std::cout << "   Note: " << all[i].note << std::endl;
        std::cout << "   Date: " << all[i].date << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
