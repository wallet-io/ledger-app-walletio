#pragma once

#include "os.h"
#include "cx.h"

#define MAX_CHARS_TITLE 32
#define MAX_CHARS_PER_KEY_LINE 64
#define MAX_CHARS_PER_VALUE_LINE 130
#define MAX_SCREEN_LINE_WIDTH 22

enum UI_DISPLAY_MODE {
    VALUE_SCROLLING,
    KEY_SCROLLING_NO_VALUE,
    KEY_SCROLLING_SHORT_VALUE,
    PENDING
};

// Index of the currently displayed page
extern int viewctl_DetailsCurrentPage;
// Total number of displayable pages
extern int viewctl_DetailsPageCount;

// Below data is used to help split long messages that are scrolled
// into smaller chunks so they fit the memory buffer
// Index of currently displayed value chunk
extern int viewctl_ChunksIndex;
// Total number of displayable value chunks
extern int viewctl_ChunksCount;

// Delegate to update contents
typedef int (*viewctl_delegate_getData)(char *title, int max_title_length,
                                        char *key, int max_key_length,
                                        char *value, int max_value_length,
                                        int page_index, int chunk_index,
                                        int *page_count_out,
                                        int *chunk_count_out);

// Delegate to handle exit view event
typedef void (*viewctl_delegate_exit)(unsigned int ignored);

// Delegate to handle exit view event
typedef void (*viewctl_delegate_ready)(unsigned int ignored);

// Delegate to handle exit view event
typedef void (*viewctl_delegate_display_ux)(int page, int count);

// Delegate to handle an accept event
typedef void (*viewctl_delegate_accept)();

// Delegate to handle a reject event
typedef void (*viewctl_delegate_reject)();

extern viewctl_delegate_getData viewctl_ehGetData;
extern viewctl_delegate_ready viewctl_ehReady;
extern viewctl_delegate_exit viewctl_ehExit;
extern viewctl_delegate_display_ux viewctl_display_ux;

// void viewctl_start(int start_page,
//                    bool single_page,
//                    viewctl_delegate_getData func_getData,
//                    viewctl_delegate_ready ehReady,
//                    viewctl_delegate_exit ehExit,
//                    viewctl_delegate_display_ux func_display_ux);

void viewctl_display_page();

void submenu_left();

void submenu_right();

void menu_left();

void menu_right();
