#include "search_view.h"
#include "reader/draw_modal_border.h"
#include "reader/system_styling.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "sys/keymap.h"

#include <SDL/SDL.h>

SearchView::SearchView(SystemStyling &styling, Callback on_submit, std::string initial)
    : styling(styling), on_submit(on_submit), query(std::move(initial))
{
    styling_sub_id = styling.subscribe_to_changes([this](SystemStyling::ChangeId) { _needs_render = true; });
}

SearchView::~SearchView()
{
    styling.unsubscribe_from_changes(styling_sub_id);
}

bool SearchView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (!_needs_render && !force_render) return false;

    TTF_Font *font = cached_load_font("default", styling.get_font_size());
    const auto &theme = styling.get_loaded_color_theme();

    std::string prompt = "Search: " + query + "\n(Enter to search, Backspace to edit, Esc to cancel)";

    auto text = surface_unique_ptr { TTF_RenderUTF8_Shaded(font, prompt.c_str(), theme.main_text, theme.background) };

    draw_modal_border(text->w, text->h, styling.get_loaded_color_theme(), dest_surface);

    SDL_Rect rect = { static_cast<Sint16>(SCREEN_WIDTH / 2 - text->w / 2), static_cast<Sint16>(SCREEN_HEIGHT / 2 - text->h / 2), 0, 0 };
    SDL_BlitSurface(text.get(), NULL, dest_surface, &rect);

    _needs_render = false;

    return true;
}

bool SearchView::is_done()
{
    return _is_done;
}

void SearchView::on_keypress(SDLKey key)
{
    // Basic text input using key presses for letters, numbers, space and simple punctuation
    if (key >= SDLK_a && key <= SDLK_z) {
        char c = 'a' + (key - SDLK_a);
        query.push_back(c);
        _needs_render = true;
        return;
    }
    if (key >= SDLK_0 && key <= SDLK_9) {
        char c = '0' + (key - SDLK_0);
        query.push_back(c);
        _needs_render = true;
        return;
    }
    if (key == SDLK_SPACE) {
        query.push_back(' ');
        _needs_render = true;
        return;
    }

    switch (key) {
        case SDLK_BACKSPACE:
            if (!query.empty()) { query.pop_back(); _needs_render = true; }
            break;
        case SDLK_RETURN:
        case SW_BTN_START:
            // Submit
            on_submit(query);
            _is_done = true;
            break;
        case SDLK_ESCAPE:
        case SW_BTN_MENU:
            // Cancel
            on_submit(std::string());
            _is_done = true;
            break;
        default:
            break;
    }
}
