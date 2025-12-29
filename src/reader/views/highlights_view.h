#ifndef HIGHLIGHTS_VIEW_H
#define HIGHLIGHTS_VIEW_H

#include "reader/view.h"
#include "highlight_manager.h"
#include <vector>
#include <functional>

class SystemStyling;

class HighlightsView : public View {
public:
    using Callback = std::function<void(int highlight_index)>;
    
    HighlightsView(SystemStyling &styling, Callback on_select);
    ~HighlightsView() override;
    
    bool render(SDL_Surface *dest, bool force) override;
    void on_keypress(SDLKey key) override;
    bool is_done() override;

private:
    SystemStyling &styling;
    Callback on_select;
    std::vector<Highlight> highlights;
    int selected_index = 0;
    bool _is_done = false;
    bool _needs_render = true;
    uint32_t styling_sub_id;
};

#endif
