#ifndef SEARCH_VIEW_H_
#define SEARCH_VIEW_H_

#include "reader/view.h"
#include "reader/system_styling.h"
#include <functional>
#include <string>

class SearchView : public View
{
public:
    using Callback = std::function<void(const std::string&)>;

    SearchView(SystemStyling &styling, Callback on_submit, std::string initial = "");
    ~SearchView();

    bool render(SDL_Surface *dest, bool force_render) override;
    bool is_done() override;
    bool is_modal() override { return true; }

    void on_keypress(SDLKey key) override;

private:
    SystemStyling &styling;
    Callback on_submit;
    std::string query;
    bool _is_done = false;
    uint32_t styling_sub_id;
    bool _needs_render = true;
};

#endif
