#include "highlights_view.h"
#include "reader/system_styling.h"
#include "reader/draw_modal_border.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "sys/keymap.h"
#include <SDL/SDL.h>

HighlightsView::HighlightsView(SystemStyling &styling, Callback on_select)
    : styling(styling), on_select(on_select)
{
    // Load all highlights
    HighlightManager mgr;
    highlights = mgr.load_all_highlights();
    
    styling_sub_id = styling.subscribe_to_changes([this](SystemStyling::ChangeId) { 
        _needs_render = true; 
    });
}

HighlightsView::~HighlightsView() {
    styling.unsubscribe_from_changes(styling_sub_id);
}

bool HighlightsView::render(SDL_Surface *dest, bool force) {
    if (!_needs_render && !force) return false;
    
    const auto &theme = styling.get_loaded_color_theme();
    TTF_Font *font = cached_load_font("default", styling.get_font_size());
    
    // Clear screen
    SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 
        theme.background.r, theme.background.g, theme.background.b));
    
    // Title
    auto title = surface_unique_ptr{
        TTF_RenderUTF8_Shaded(font, "MY HIGHLIGHTS", theme.main_text, theme.background)
    };
    SDL_Rect title_rect = {20, 20, 0, 0};
    SDL_BlitSurface(title.get(), NULL, dest, &title_rect);
    
    // Show highlights
    int y = 60;
    for (size_t i = 0; i < highlights.size() && y < SCREEN_HEIGHT - 40; i++) {
        std::string line;
        
        // Add brackets if selected
        if (i == static_cast<size_t>(selected_index)) {
            line = "[ \"" + highlights[i].text.substr(0, 40) + "...\" ]";
        } else {
            line = "  \"" + highlights[i].text.substr(0, 40) + "...\"";
        }
        
        auto text = surface_unique_ptr{
            TTF_RenderUTF8_Shaded(font, line.c_str(), theme.main_text, theme.background)
        };
        SDL_Rect rect = {20, static_cast<Sint16>(y), 0, 0};
        SDL_BlitSurface(text.get(), NULL, dest, &rect);
        
        y += 30;
    }
    
    // Instructions
    auto help = surface_unique_ptr{
        TTF_RenderUTF8_Shaded(font, "A: View  B: Back", theme.main_text, theme.background)
    };
    SDL_Rect help_rect = {20, SCREEN_HEIGHT - 40, 0, 0};
    SDL_BlitSurface(help.get(), NULL, dest, &help_rect);
    
    _needs_render = false;
    return true;
}

void HighlightsView::on_keypress(SDLKey key) {
    if (key == SW_BTN_B) {
        _is_done = true;
        return;
    }
    
    if (key == SW_BTN_UP && selected_index > 0) {
        selected_index--;
        _needs_render = true;
    }
    
    if (key == SW_BTN_DOWN && selected_index < static_cast<int>(highlights.size()) - 1) {
        selected_index++;
        _needs_render = true;
    }
    
    if (key == SW_BTN_A && !highlights.empty()) {
        on_select(selected_index);
    }
}

bool HighlightsView::is_done() {
    return _is_done;
}
