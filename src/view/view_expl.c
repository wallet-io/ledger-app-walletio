#include "view_expl.h"
#include "common.h"
#include "view_templates.h"

#include "bagl.h"
#include "glyphs.h"
#include "zxmacros.h"

#include <stdio.h>
#include <string.h>

extern volatile char viewctl_DataKey[MAX_CHARS_PER_KEY_LINE];
extern volatile char viewctl_DataValue[MAX_CHARS_PER_VALUE_LINE];
extern volatile char viewctl_Title[MAX_SCREEN_LINE_WIDTH];
extern enum UI_DISPLAY_MODE viewctl_scrolling_mode;
extern int viewctl_accept_page_index;
extern int viewctl_reject_page_index;

static const bagl_element_t viewexpl_bagl_valuescrolling_first[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataKey),
    UI_LabelLineScrolling(2, 16, 30, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataValue),
};

static const bagl_element_t viewexpl_bagl_valuescrolling[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataKey),
    UI_LabelLineScrolling(2, 16, 30, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataValue),
};

static const bagl_element_t viewexpl_bagl_valuescrolling_last[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
    UI_Icon(0, 128 - 10, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataKey),
    UI_LabelLineScrolling(2, 16, 30, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataValue),
};

static const bagl_element_t viewexpl_bagl_valuescrolling_only[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 128 - 10, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataKey),
    UI_LabelLineScrolling(2, 16, 30, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataValue),
};

// -----

static const bagl_element_t viewexpl_bagl_keyscrolling_first[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataValue),
    UI_LabelLineScrolling(2, 16, 19, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataKey),
};

static const bagl_element_t viewexpl_bagl_keyscrolling[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_RIGHT),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataValue),
    UI_LabelLineScrolling(2, 16, 19, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataKey),
};

static const bagl_element_t viewexpl_bagl_keyscrolling_last[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_LEFT),
    UI_Icon(0, 128 - 10, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataValue),
    UI_LabelLineScrolling(2, 16, 19, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataKey),
};

static const bagl_element_t viewexpl_bagl_keyscrolling_only[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 128 - 10, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_Title),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000,
                 (const char *)viewctl_DataValue),
    UI_LabelLineScrolling(2, 16, 19, 96, 11, 0xFFFFFF, 0x000000,
                          (const char *)viewctl_DataKey),
};

// -----

const bagl_element_t *viewexpl_bagl_prepro(const bagl_element_t *element) {
    switch (element->component.userid) {
        case 0x01:
            UX_CALLBACK_SET_INTERVAL(2000);
            break;
        case 0x02:
            UX_CALLBACK_SET_INTERVAL(
                MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
        case 0x03:
            UX_CALLBACK_SET_INTERVAL(
                MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
    }
    return element;
}

static unsigned int viewexpl_bagl_keyscrolling_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    switch (button_mask) {
        // Press both left and right to switch to value scrolling
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            if (viewctl_DetailsCurrentPage == viewctl_accept_page_index) {
                tx_info_accept(0);
            }
            if (viewctl_DetailsCurrentPage == viewctl_reject_page_index) {
                tx_info_reject(0);
            }
            break;
        }

            // Press left to progress to the previous element
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            if (viewctl_ChunksIndex > 0) {
                submenu_left();
            } else {
                menu_left();
            }
            break;
        }

            // Hold left to progress to the previous element quickly
            // It also steps out from the value chunk page view mode
        case BUTTON_EVT_FAST | BUTTON_LEFT: {
            menu_left();
            break;
        }

            // Press right to progress to the next element
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            if (viewctl_ChunksIndex < viewctl_ChunksCount - 1) {
                submenu_right();
            } else {
                menu_right();
            }
            break;
        }

            // Hold right to progress to the next element quickly
            // It also steps out from the value chunk page view mode
        case BUTTON_EVT_FAST | BUTTON_RIGHT: {
            menu_right();
            break;
        }
    }
    return 0;
}

static unsigned int viewexpl_bagl_keyscrolling_first_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_keyscrolling_button(button_mask, button_mask_counter);
}

static unsigned int viewexpl_bagl_keyscrolling_last_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_keyscrolling_button(button_mask, button_mask_counter);
}

static unsigned int viewexpl_bagl_keyscrolling_only_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_keyscrolling_button(button_mask, button_mask_counter);
}

static unsigned int viewexpl_bagl_valuescrolling_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_keyscrolling_button(button_mask, button_mask_counter);
}

static unsigned int viewexpl_bagl_valuescrolling_first_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_valuescrolling_button(button_mask,
                                               button_mask_counter);
}

static unsigned int viewexpl_bagl_valuescrolling_last_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_valuescrolling_button(button_mask,
                                               button_mask_counter);
}

static unsigned int viewexpl_bagl_valuescrolling_only_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    return viewexpl_bagl_valuescrolling_button(button_mask,
                                               button_mask_counter);
}

void viewexpl_display_ux(int page, int count) {
    if (viewctl_scrolling_mode == VALUE_SCROLLING) {
        if (page == 0) {
            UX_DISPLAY(viewexpl_bagl_valuescrolling_first,
                       viewexpl_bagl_prepro);
            if (count - 1 <= page) {
                UX_DISPLAY(viewexpl_bagl_valuescrolling_only,
                           viewexpl_bagl_prepro);
            }
        } else if (count - 1 <= page) {
            UX_DISPLAY(viewexpl_bagl_valuescrolling_last, viewexpl_bagl_prepro);
        } else {
            UX_DISPLAY(viewexpl_bagl_valuescrolling, viewexpl_bagl_prepro);
        }
    } else if (page == 0) {
        UX_DISPLAY(viewexpl_bagl_keyscrolling_first, viewexpl_bagl_prepro);
        if (count - 1 <= page) {
            UX_DISPLAY(viewexpl_bagl_keyscrolling_only, viewexpl_bagl_prepro);
        }
    } else if (count - 1 <= page) {
        UX_DISPLAY(viewexpl_bagl_keyscrolling_last, viewexpl_bagl_prepro);
    } else {
        UX_DISPLAY(viewexpl_bagl_keyscrolling, viewexpl_bagl_prepro);
    }
}

void viewexpl_start(int start_page, bool single_page,
                    viewctl_delegate_getData ehUpdate,
                    viewctl_delegate_ready ehReady,
                    viewctl_delegate_exit ehExit) {
    viewctl_start(start_page, single_page, ehUpdate, ehReady, ehExit,
                  viewexpl_display_ux);
}
