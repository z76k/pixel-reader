#include "./reader_view.h"
#include "util/sdl_pointer.h"
#include "highlight.h"  
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

struct ReaderViewState
{
    bool is_done = false;
    std::function<void(DocAddr)> on_change_address;

    std::string filename;
    std::shared_ptr<DocReader> reader;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    uint32_t token_view_styling_sub_id;

    ViewStack &view_stack;
    std::unique_ptr<TokenView> token_view;

    // Search state
    std::string search_query;
    std::vector<DocAddr> search_results;
    size_t current_search_index = 0;

    ReaderViewState(std::filesystem::path path, DocAddr seek_address, std::shared_ptr<DocReader> reader, SystemStyling &sys_styling, TokenViewStyling &token_view_styling, uint32_t token_view_styling_sub_id, ViewStack &view_stack)
        : filename(path.filename().string()),
          reader(reader),
          sys_styling(sys_styling),
          token_view_styling(token_view_styling),
          token_view_styling_sub_id(token_view_styling_sub_id),
          view_stack(view_stack),
          token_view(std::make_unique<TokenView>(
              reader,
              seek_address,
              sys_styling,
              token_view_styling
          ))
    {
        // --- 1. INITIALIZE HIGHLIGHTS ---
        currentBookFile = filename; 
        loadHighlights();
    }

    ~ReaderViewState() {}
};

namespace
{
DocAddr get_current_address(const ReaderViewState &state)
{
    return state.token_view ? state.token_view->get_address() : 0;
}

static std::string to_lower(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(std::tolower(c));
    return out;
}

void perform_search(ReaderViewState &state, const std::string &query)
{
    state.search_query = query;
    state.search_results.clear();
    state.current_search_index = 0;

    if (query.empty()) return;

    auto iter = state.reader->get_iter(0);
    while (const DocToken *tok = iter->read(1)) {
        // Only search text-bearing tokens
        std::string txt = tok->to_string();
        if (txt.empty()) continue;
        std::string lower_txt = to_lower(txt);
        std::string lower_query = to_lower(query);
        if (lower_txt.find(lower_query) != std::string::npos) {
            state.search_results.push_back(tok->address);
        }
    }
}

void go_to_search_index(ReaderViewState &state, size_t index)
{
    if (index >= state.search_results.size()) return;
    state.current_search_index = index;
    DocAddr addr = state.search_results[index];
    if (state.token_view) {
        state.token_view->seek_to_address(addr);
        state.token_view->set_title_progress(0);
    }
}

void open_toc_menu(ReaderView &reader_view, ReaderViewState &state) { /* ... keep original TOC code here ... */ }

void open_highlights_menu(ReaderView &reader_view, ReaderViewState &state)
{
    auto hl_list = getHighlightsForCurrentBook();
    std::vector<std::string> entries;
    for (size_t i = 0; i < hl_list.size(); ++i) {
        const auto &h = hl_list[i];
        entries.push_back(std::string("Highlight ") + std::to_string(i+1) + ": " + std::to_string(h.start) + " - " + std::to_string(h.end));
    }

    if (entries.empty()) {
        entries.push_back("No highlights");
    }

    auto menu = std::make_shared<SelectionMenu>(entries, state.sys_styling);
    menu->set_close_on_select();
    menu->set_on_selection([&state, menu](uint32_t idx) {
        auto hlist = getHighlightsForCurrentBook();
        if (hlist.empty()) return;
        if (idx >= hlist.size()) return;

        // Action menu for a highlight
        auto action_entries = std::vector<std::string>{"Jump to highlight", "Delete highlight", "Cancel"};
        auto action_menu = std::make_shared<SelectionMenu>(action_entries, state.sys_styling);
        action_menu->set_on_selection([&state, idx](uint32_t a) {
            auto hlist2 = getHighlightsForCurrentBook();
            if (idx >= hlist2.size()) return;
            if (a == 0) {
                // Jump
                if (state.token_view) {
                    state.token_view->seek_to_address((DocAddr)hlist2[idx].start);
                }
            } else if (a == 1) {
                // Delete
                deleteHighlightAt(idx);
                loadHighlights();
            }
        });
        state.view_stack.push(action_menu);
    });

    state.view_stack.push(menu);
}

void open_search_results_menu(ReaderView &reader_view, ReaderViewState &state)
{
    std::vector<std::string> entries;
    for (size_t i = 0; i < state.search_results.size(); ++i) {
        DocAddr addr = state.search_results[i];
        auto iter = state.reader->get_iter(addr);
        const DocToken *tok = iter->read(1);
        std::string snippet = tok ? tok->to_string() : std::string("(no text)");
        if (snippet.size() > 60) snippet = snippet.substr(0, 57) + "...";
        entries.push_back(std::string("Result ") + std::to_string(i+1) + ": " + snippet);
    }

    if (entries.empty()) entries.push_back("No results");

    auto menu = std::make_shared<SelectionMenu>(entries, state.sys_styling);
    menu->set_close_on_select();
    menu->set_on_selection([&state](uint32_t idx) {
        if (idx >= state.search_results.size()) return;
        go_to_search_index(state, idx);
    });

    state.view_stack.push(menu);
}
} 

ReaderView::ReaderView(std::filesystem::path path, std::shared_ptr<DocReader> reader, DocAddr seek_address, SystemStyling &sys_styling, TokenViewStyling &token_view_styling, ViewStack &view_stack) 
    : state(std::make_unique<ReaderViewState>(path, seek_address, reader, sys_styling, token_view_styling, token_view_styling.subscribe_to_changes([this]() { update_token_view_title(get_current_address(*state)); }), view_stack))
{
    update_token_view_title(seek_address);
    state->token_view->set_on_scroll([this](DocAddr address) {
        update_token_view_title(address);
        if (state->on_change_address) state->on_change_address(address);
    });
}

ReaderView::~ReaderView() {
    state->token_view_styling.unsubscribe_from_changes(state->token_view_styling_sub_id);
}

// --- 2. THE RENDER HOOK ---
bool ReaderView::render(SDL_Surface *dest_surface, bool force_render)
{
    // First, let the system draw the text
    bool rendered = state->token_view->render(dest_surface, force_render);

    // Now, if we are currently highlighting, draw a "Status" box at the top
    if (selecting) {
        SDL_Rect statusBox = { 5, 5, 10, 10 }; // Small yellow square in corner
        SDL_FillRect(dest_surface, &statusBox, SDL_MapRGB(dest_surface->format, 255, 255, 0));
    }

    // Show search status if available
    if (!state->search_query.empty() && !state->search_results.empty()) {
        TTF_Font *font = cached_load_font("default", state->sys_styling.get_font_size());
        const auto &theme = state->sys_styling.get_loaded_color_theme();
        size_t total = state->search_results.size();
        size_t idx = state->current_search_index + 1; // human 1-based
        std::string status = "Search: '" + state->search_query + "' (" + std::to_string(idx) + "/" + std::to_string(total) + ")";
        auto text = surface_unique_ptr { TTF_RenderUTF8_Shaded(font, status.c_str(), theme.main_text, theme.background) };
        SDL_Rect rect = { 20, 5, 0, 0 };
        SDL_BlitSurface(text.get(), NULL, dest_surface, &rect);
    }
    
    return rendered;
}

void ReaderView::on_keypress(SDLKey key)
{
    if (key == SW_BTN_B) {
        state->is_done = true;
        return;
    }

    switch (key) {
        // --- 3. THE BUTTON HOOK ---
        case SW_BTN_X: // Let's use the X button for highlights so we don't break the A button menu!
            {
                DocAddr addr = get_current_address(*state);
                startOrFinishHighlight((int)addr);
            }
            break;

        case SW_BTN_Y: // Open search or cycle to next match
            {
                // If there's already a search running, cycle to next result
                if (!state->search_query.empty() && !state->search_results.empty()) {
                    size_t next = (state->current_search_index + 1) % state->search_results.size();
                    go_to_search_index(*state, next);
                    break;
                }

                auto on_submit = [this](const std::string &q) {
                    if (q.empty()) return; // cancelled
                    perform_search(*state, q);
                    if (!state->search_results.empty()) {
                        go_to_search_index(*state, 0);
                        // Open a list UI for results so the user can pick
                        open_search_results_menu(*this, *state);
                    } else {
                        if (state->token_view) {
                            state->token_view->set_title("No matches");
                        }
                    }
                };
                state->view_stack.push(std::make_shared<SearchView>(state->sys_styling, on_submit, state->search_query));
            }
            break;

        case SW_BTN_A:
            state->token_view_styling.set_show_title_bar(!state->token_view_styling.get_show_title_bar());
            break;

        case SW_BTN_L1:
            // Previous search result
            if (!state->search_results.empty()) {
                size_t total = state->search_results.size();
                size_t prev = (state->current_search_index + total - 1) % total;
                go_to_search_index(*state, prev);
            }
            break;
        case SW_BTN_R1:
            // Next search result
            if (!state->search_results.empty()) {
                size_t total = state->search_results.size();
                size_t next = (state->current_search_index + 1) % total;
                go_to_search_index(*state, next);
            }
            break;

        case SW_BTN_SELECT: open_toc_menu(*this, *state); break;
            open_toc_menu(*this, *state);
            break;

        case SW_BTN_MENU:
            {
                // Mini menu: Search Results / Highlights / Cancel
                std::vector<std::string> entries = {"Search Results", "Highlights", "Cancel"};
                auto menu = std::make_shared<SelectionMenu>(entries, state->sys_styling);
                menu->set_on_selection([this](uint32_t idx) {
                    if (idx == 0) {
                        open_search_results_menu(*this, *state);
                    } else if (idx == 1) {
                        open_highlights_menu(*this, *state);
                    }
                });
                state->view_stack.push(menu);
            }
            break;
        default:
            state->token_view->on_keypress(key);
            break;
    }
}

// ... (Rest of the helper functions: is_done, update_token_view_title, etc.)
void ReaderView::update_token_view_title(DocAddr address) { /* ... keep original ... */ }
bool ReaderView::is_done() { return state->is_done; }
void ReaderView::on_keyheld(SDLKey key, uint32_t hold_time_ms) { state->token_view->on_keyheld(key, hold_time_ms); }
void ReaderView::set_on_change_address(std::function<void(DocAddr)> callback) { state->on_change_address = callback; }
void ReaderView::seek_to_toc_index(uint32_t toc_index) { seek_to_address(state->reader->get_toc_item_address(toc_index)); }
void ReaderView::seek_to_address(DocAddr address) { if (state->token_view) { state->token_view->seek_to_address(address); update_token_view_title(address); if (state->on_change_address) state->on_change_address(address); } }