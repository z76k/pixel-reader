#include <iostream>
bool selecting = false;
void startOrFinishHighlight(int dummy) {
    if (!selecting) {
        selecting = true;
    } else {
        selecting = false;
    }
}
