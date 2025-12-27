#include "./reader_view.h"
#include "util/sdl_pointer.h"
#include "search_view.h"
#include "./selection_menu.h"
#include "./token_view/token_view.h"
#include "./token_view/token_view_styling.h"
#include "reader/system_styling.h"
#include "reader/view_stack.h"
#include "doc_api/doc_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>

struct HighlightEntry {
    DocAddr start;
    DocAddr end;
};

struct ReaderViewState {
    bool is_done = false;
    std::function<void(DocAddr)> on_change_address;
    std::string filename;
    std::shared_ptr<DocReader> reader;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    uint32_t token_view_styling_sub_id;
    ViewStack &view_stack;
    std::unique_ptr<TokenView> token_view;

    // Highlight State
    bool selecting = false;
    DocAddr highlight_start_addr = 0;
    std::vector<HighlightEntry> highlights;

    // Search state
    std::string search_query;
    std::vector<DocAddr> search_results;
    size_t current_search_index = 0;

    ReaderViewState(std::filesystem::path path, DocAddr seek_address, std::shared_ptr<DocReader> reader, SystemStyling &sys_styling, TokenViewStyling &token_view_styling, uint32_t token_view_styling_sub_id, ViewStack &view_stack)
        : filename(path.filename().string()), reader(reader), sys_styling(sys_styling),
          token_view_styling(token_view_styling), token_view_styling_sub_id(token_view_styling_sub_id),
          view_stack(view_stack),
          token_view(std::make_unique<TokenView>(reader, seek_address, sys_styling, token_view_styling))
    {}
};

namespace {
    void save_hl_safe(const ReaderViewState &state, DocAddr s, DocAddr e) {
        // Saving to the current working directory is safest on Linux
        std::ofstream f("highlights_database.txt", std::ios::app);
        if (f.is_open()) {
            f << "File: " << state.filename << " | Addr: " << s << " to " << e << "\n";
            f.close();
        }
    }
}

ReaderView::ReaderView(std::filesystem::path path, std::shared_ptr<DocReader> reader, DocAddr seek_address, SystemStyling &sys_styling, TokenViewStyling &token_view_styling, ViewStack &view_stack) 
    : state(std::make_unique<ReaderViewState>(path, seek_address, reader, sys_styling, token_view_styling, 0, view_stack))
{
    state->token_view->set_on_scroll([this](DocAddr address) {
        if (state->on_change_address) state->on_change_address(address);
    });
}

ReaderView::~ReaderView() {}

bool ReaderView::render(SDL_Surface *dest, bool force) {
    bool r = state->token_view->render(dest, force);
    if (state->selecting) {
        SDL_Rect bar = { 0, 0, (Uint16)dest->w, (Uint16)8 };
        SDL_FillRect(dest, &bar, SDL_MapRGB(dest->format, 255, 255, 0));
    }
    return r;
}

void ReaderView::on_keypress(SDLKey key) {
    if (key == SW_BTN_B) { state->is_done = true; return; }
    switch (key) {
        case SW_BTN_Y:
            {
                DocAddr cur = state->token_view->get_address();
                if (!state->selecting) {
                    state->selecting = true;
                    state->highlight_start_addr = cur;
                } else {
                    state->selecting = false;
                    save_hl_safe(*state, state->highlight_start_addr, cur);
                }
            }
            break;
        case SW_BTN_A:
            state->token_view_styling.set_show_title_bar(!state->token_view_styling.get_show_title_bar());
            break;
        default:
            state->token_view->on_keypress(key);
            break;
    }
}

bool ReaderView::is_done() { return state->is_done; }
void ReaderView::update_token_view_title(DocAddr /* address */) {}
void ReaderView::on_keyheld(SDLKey key, uint32_t ms) { state->token_view->on_keyheld(key, ms); }
void ReaderView::set_on_change_address(std::function<void(DocAddr)> cb) { state->on_change_address = cb; }
void ReaderView::seek_to_address(DocAddr addr) { if (state->token_view) state->token_view->seek_to_address(addr); }
