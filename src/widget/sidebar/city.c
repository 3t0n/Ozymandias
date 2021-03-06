#include <tgmath.h>
#include "city.h"

#include "building/menu.h"
#include "city/message.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/direction.h"
#include "core/game_environment.h"
#include "game/orientation.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/orientation.h"
#include "scenario/property.h"
#include "widget/city.h"
#include "widget/minimap.h"
#include "widget/sidebar/common.h"
#include "widget/sidebar/extra.h"
#include "widget/sidebar/slide.h"
#include "window/advisors.h"
#include "window/build_menu.h"
#include "window/city.h"
#include "window/empire.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_briefing.h"
#include "window/overlay_menu.h"

#define MINIMAP_Y_OFFSET 59

static void button_overlay(int param1, int param2);
static void button_collapse_expand(int param1, int param2);
static void button_build(int submenu, int param2);
static void button_undo(int param1, int param2);
static void button_messages(int param1, int param2);
static void button_help(int param1, int param2);
static void button_go_to_problem(int param1, int param2);
static void button_advisors(int param1, int param2);
static void button_empire(int param1, int param2);
static void button_mission_briefing(int param1, int param2);
static void button_rotate_north(int param1, int param2);
static void button_rotate(int clockwise, int param2);

static image_button buttons_overlays_collapse_sidebar[][2] = {
        {
                {127, 5, 31, 20, IB_NORMAL, 90,  0, button_collapse_expand, button_none, 0, 0, 1},
                {4, 3, 117, 31, IB_NORMAL, 93, 0, button_overlay, button_help, 0, MESSAGE_DIALOG_OVERLAYS, 1}
        },
        {
                {128, 0, 31, 20, IB_NORMAL, 176, 7, button_collapse_expand, button_none, 0, 0, 1},
//        {4 - 15 - 5, 2, 117, 20, IB_NORMAL, 93, 0, button_overlay, button_help, 0, MESSAGE_DIALOG_OVERLAYS, 1}
        }
};

static image_button button_expand_sidebar[][1] = {
        {
                {6, 4, 31, 20, IB_NORMAL, 90,  4,  button_collapse_expand, button_none, 0, 0, 1}
        },
        {
                {8, 0, 31, 20, IB_NORMAL, 176, 10, button_collapse_expand, button_none, 0, 0, 1}
        }
};

#define CL_ROW0 21 //22

static image_button buttons_build_collapsed[][20] = {
        {
                {2, 32,      39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 0,  button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
                {2, 67, 39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 8, button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},
                {2, 102, 39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 12, button_build, button_none, BUILD_MENU_ROAD, 0, 1},
                {2, 137, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 4, button_build, button_none, BUILD_MENU_WATER, 0, 1},
                {2, 172, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 40, button_build, button_none, BUILD_MENU_HEALTH, 0, 1},
                {2, 207, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 28, button_build, button_none, BUILD_MENU_RELIGION, 0, 1},
                {2, 242, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 24, button_build, button_none, BUILD_MENU_EDUCATION, 0, 1},
                {2, 277, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 20, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
                {2, 312, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 16, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
                {2, 347, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 44, button_build, button_none, BUILD_MENU_ENGINEERING, 0, 1},
                {2, 382, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 36, button_build, button_none, BUILD_MENU_SECURITY, 0, 1},
                {2, 417, 39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 32, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
        },
        {
                {9, CL_ROW0, 36, 48, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 90, button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
                {9, CL_ROW0 +
                    36, 36, 48, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 94, button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},
                {9, CL_ROW0 +
                    71, 36, 48, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 98, button_build, button_none, BUILD_MENU_ROAD, 0, 1},
                {9, CL_ROW0 +
                    108, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 102, button_build, button_none, BUILD_MENU_WATER, 0, 1},
                {9, CL_ROW0 +
                    142, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 106, button_build, button_none, BUILD_MENU_HEALTH, 0, 1},
                {9, CL_ROW0 +
                    177, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 110, button_build, button_none, BUILD_MENU_RELIGION, 0, 1},
                {9, CL_ROW0 +
                    212, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 114, button_build, button_none, BUILD_MENU_EDUCATION, 0, 1},
                {9, CL_ROW0 +
                    245, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 118, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
                {9, CL_ROW0 +
                    281, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 122, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
                {9, CL_ROW0 +
                    317, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 126, button_build, button_none, BUILD_MENU_ENGINEERING, 0, 1},
                {9, CL_ROW0 +
                    353, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 130, button_build, button_none, BUILD_MENU_SECURITY, 0, 1},
                {9, CL_ROW0 +
                    385, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 134, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
        }
};

#define COL1 9
#define COL2 COL1 + 34 + 3
#define COL3 COL2 + 36 + 4
#define COL4 COL3 + 34 + 5

#define ROW1 251
#define ROW2 ROW1 + 48 + 1
#define ROW3 ROW2 + 50 + 1
#define ROW4 ROW3 + 49 + 4

static image_button buttons_build_expanded[][20] = {
        {
                {13, 277,    39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 0, button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
                {63, 277,    39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 8, button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},
                {113, 277,   39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 12, button_build, button_none, BUILD_MENU_ROAD,       0, 1},
                {13, 313,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 4,  button_build, button_none, BUILD_MENU_WATER, 0, 1},
                {63, 313,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 40, button_build, button_none, BUILD_MENU_HEALTH,   0, 1},
                {113, 313,   39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 28, button_build, button_none, BUILD_MENU_RELIGION,     0, 1},
                {13, 349,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 24, button_build, button_none, BUILD_MENU_EDUCATION,     0, 1},
                {63, 349,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 20, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
                {113, 349,   39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 16, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
                {13, 385,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 44, button_build, button_none, BUILD_MENU_ENGINEERING, 0, 1},
                {63, 385,    39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 36, button_build, button_none, BUILD_MENU_SECURITY,       0, 1},
                {113, 385,   39, 26, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 32, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
                {13, 421,    39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 48, button_undo, button_none, 0, 0, 1},
                {63, 421,    39, 26, IB_NORMAL, GROUP_MESSAGE_ICON,    18, button_messages, button_help, 0, MESSAGE_DIALOG_MESSAGES, 1},
                {113, 421,   39, 26, IB_BUILD, GROUP_MESSAGE_ICON,    22, button_go_to_problem, button_none, 0, 0, 1},
        },
        {
                {COL1, ROW1, 34, 48, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 0, button_build, button_none, BUILD_MENU_VACANT_HOUSE, 0, 1},
                {COL1, ROW2, 34, 50, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 4, button_build, button_none, BUILD_MENU_ROAD,       0, 1},
                {COL1, ROW3, 34, 49, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 8,  button_build, button_none, BUILD_MENU_CLEAR_LAND, 0, 1},

                {COL2, ROW1, 36, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 12, button_build, button_none, BUILD_MENU_FOOD,  0, 1},
                {COL2, ROW2, 36, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 16, button_build, button_none, BUILD_MENU_INDUSTRY, 0, 1},
                {COL2, ROW3, 36, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 20, button_build, button_none, BUILD_MENU_DISTRIBUTION, 0, 1},

                {COL3, ROW1, 34, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 24, button_build, button_none, BUILD_MENU_ENTERTAINMENT, 0, 1},
                {COL3, ROW2, 34, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 28, button_build, button_none, BUILD_MENU_RELIGION,      0, 1},
                {COL3, ROW3, 34, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 32, button_build, button_none, BUILD_MENU_EDUCATION,      0, 1},

                {COL4, ROW1, 34, 48, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 36, button_build, button_none, BUILD_MENU_HEALTH,      0, 1},
                {COL4, ROW2, 34, 50, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 40, button_build, button_none, BUILD_MENU_ADMINISTRATION, 0, 1},
                {COL4, ROW3, 34, 49, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 44, button_build, button_none, BUILD_MENU_SECURITY, 0, 1},

                {COL1, ROW4, 35, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 48, button_undo, button_none, 0, 0, 1},
                {COL2, ROW4, 38, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 52, button_messages, button_help, 0, MESSAGE_DIALOG_MESSAGES, 1},
                {COL3, ROW4, 28, 45, IB_BUILD, GROUP_SIDEBAR_BUTTONS, 56, button_go_to_problem, button_none, 0, 0, 1},
//        {COL4 - 9, ROW4, 43, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 60, button_mission_briefing, button_none, 0, 0, 1},
        }
};

static image_button buttons_top_expanded[][10] = {
        {
                {7, 155, 71, 23, IB_NORMAL, GROUP_SIDEBAR_ADVISORS_EMPIRE, 0, button_advisors, button_none, 0, 0, 1},
                {84, 155, 71, 23, IB_NORMAL, GROUP_SIDEBAR_ADVISORS_EMPIRE, 3, button_empire, button_help, 0, MESSAGE_DIALOG_EMPIRE_MAP, 1},
                {7, 184,   33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 0, button_mission_briefing, button_none, 0, 0, 1},
                {46, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 3, button_rotate_north, button_none, 0, 0, 1},
                {84, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 6, button_rotate, button_none, 0, 0, 1},
                {123, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 9, button_rotate, button_none, 1, 0, 1},
        },
        {
                {COL1 + 7, 143, 60, 36, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 64, button_advisors, button_none, 0, 0, 1},
                {COL3 +
                 4, 143, 62, 36, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 68, button_empire, button_help, 0, MESSAGE_DIALOG_EMPIRE_MAP, 1},
//        {COL1, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 0, button_mission_briefing, button_none, 0, 0, 1},
                {COL4 -
                 9,        ROW4, 43, 45, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 60, button_mission_briefing, button_none, 0, 0, 1},
//        {COL2, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 3, button_rotate_north, button_none, 0, 0, 1},
//        {COL3, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 6, button_rotate, button_none, 0, 0, 1},
//        {COL4, 184, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 9, button_rotate, button_none, 1, 0, 1},
        }
};

static struct {
    int focus_button_for_tooltip;
} data;

static void draw_overlay_text(int x_offset) {
    if (get_game_engine() == ENGINE_ENV_C3) {
        if (game_state_overlay())
            lang_text_draw_centered(14, game_state_overlay(), x_offset, 32, 117, FONT_NORMAL_GREEN);
        else
            lang_text_draw_centered(6, 4, x_offset, 32, 117, FONT_NORMAL_GREEN);
    }
    if (get_game_engine() == ENGINE_ENV_PHARAOH) {
        if (game_state_overlay())
            lang_text_draw_centered(14, game_state_overlay(), x_offset - 15, 30, 117, FONT_NORMAL_GREEN);
        else
            lang_text_draw_centered(6, 4, x_offset - 15, 30, 117, FONT_NORMAL_GREEN);
    }
}
static void draw_sidebar_remainder(int x_offset, bool is_collapsed) {
    int width = SIDEBAR_EXPANDED_WIDTH[get_game_engine()];
    if (is_collapsed)
        width = SIDEBAR_COLLAPSED_WIDTH;
    int available_height = sidebar_common_get_height() - SIDEBAR_MAIN_SECTION_HEIGHT;
    int extra_height = sidebar_extra_draw_background(x_offset, SIDEBAR_MAIN_SECTION_HEIGHT + TOP_MENU_HEIGHT[get_game_engine()], 162, available_height, is_collapsed, SIDEBAR_EXTRA_DISPLAY_ALL);
    sidebar_extra_draw_foreground();
    int relief_y_offset =
            SIDEBAR_MAIN_SECTION_HEIGHT + TOP_MENU_HEIGHT[0] + extra_height; // + (GAME_ENV == ENGINE_ENV_PHARAOH) * 6;
    sidebar_common_draw_relief(x_offset, relief_y_offset, GROUP_SIDE_PANEL, is_collapsed);
}
static void draw_number_of_messages(int x_offset) {
    int messages = city_message_count();
    buttons_build_expanded[get_game_engine()][13].enabled = messages > 0;
    buttons_build_expanded[get_game_engine()][14].enabled = city_message_problem_area_count();
    if (messages) {
        text_draw_number_centered_colored(messages, x_offset + 74, 452, 32, FONT_SMALL_PLAIN, COLOR_BLACK);
        text_draw_number_centered_colored(messages, x_offset + 73, 453, 32, FONT_SMALL_PLAIN, COLOR_WHITE);
    }
}

static void draw_buttons_collapsed(int x_offset) {
    image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], button_expand_sidebar[get_game_engine()], 1);
    image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_build_collapsed[get_game_engine()], 12);
}
static void draw_buttons_expanded(int x_offset) {
    buttons_build_expanded[get_game_engine()][12].enabled = game_can_undo();
    if (get_game_engine() == ENGINE_ENV_C3) {
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_overlays_collapse_sidebar[get_game_engine()], 2);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_build_expanded[get_game_engine()], 15);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_top_expanded[get_game_engine()], 6);
    } else if (get_game_engine() == ENGINE_ENV_PHARAOH) {
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_overlays_collapse_sidebar[get_game_engine()], 1);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_build_expanded[get_game_engine()], 15);
        image_buttons_draw(x_offset, TOP_MENU_HEIGHT[get_game_engine()], buttons_top_expanded[get_game_engine()], 3);
    }
}

static void refresh_build_menu_buttons(void) {
    int num_buttons = 12;
    for (int i = 0; i < num_buttons; i++) {
        buttons_build_expanded[get_game_engine()][i].enabled = 1;
        if (building_menu_count_items(buttons_build_expanded[get_game_engine()][i].parameter1) <= 0)
            buttons_build_expanded[get_game_engine()][i].enabled = 0;

        buttons_build_collapsed[get_game_engine()][i].enabled = 1;
        if (building_menu_count_items(buttons_build_collapsed[get_game_engine()][i].parameter1) <= 0)
            buttons_build_collapsed[get_game_engine()][i].enabled = 0;
    }
}
static void draw_collapsed_background(void) {
    int x_offset = sidebar_common_get_x_offset_collapsed();
    if (get_game_engine() == ENGINE_ENV_C3)
        image_draw(image_id_from_group(GROUP_SIDE_PANEL), x_offset, TOP_MENU_HEIGHT[get_game_engine()]);
    else if (get_game_engine() == ENGINE_ENV_PHARAOH)
        image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 1, x_offset, TOP_MENU_HEIGHT[get_game_engine()]);
    draw_buttons_collapsed(x_offset);
    draw_sidebar_remainder(x_offset, true);
}
static void draw_expanded_background(int x_offset) {
    if (get_game_engine() == ENGINE_ENV_C3) {
        image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 1, x_offset, TOP_MENU_HEIGHT[get_game_engine()]);
        image_draw(window_build_menu_image(), x_offset + 6, 225 + TOP_MENU_HEIGHT[get_game_engine()]);
        widget_minimap_draw(x_offset + 8, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);
        draw_number_of_messages(x_offset);
    } else if (get_game_engine() == ENGINE_ENV_PHARAOH) {
        image_draw(image_id_from_group(GROUP_SIDE_PANEL), x_offset, TOP_MENU_HEIGHT[get_game_engine()]);
        image_draw(window_build_menu_image(), x_offset + 11, 181 + TOP_MENU_HEIGHT[get_game_engine()]);
        widget_minimap_draw(x_offset + 12, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);

        // extra bar spacing on the right
        int block_height = 702;
        int s_end = 768;
        int s_num = ceil((float) (screen_height() - s_end) / (float) block_height);
        int s_start = s_num * block_height;
        for (int i = 0; i < s_num; i++)
            image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 2, x_offset + 162, s_start + i * block_height);
        image_draw(image_id_from_group(GROUP_SIDE_PANEL) + 2, x_offset + 162, 0);
        draw_number_of_messages(x_offset - 26);
    }
    draw_buttons_expanded(x_offset);
    draw_overlay_text(x_offset + 4);

    draw_sidebar_remainder(x_offset, false);
}
void widget_sidebar_city_draw_background(void) {
    if (city_view_is_sidebar_collapsed())
        draw_collapsed_background();
    else
        draw_expanded_background(sidebar_common_get_x_offset_expanded());
}
void widget_sidebar_city_draw_foreground(void) {
    if (building_menu_has_changed())
        refresh_build_menu_buttons();

    if (city_view_is_sidebar_collapsed()) {
        int x_offset = sidebar_common_get_x_offset_collapsed();
        draw_buttons_collapsed(x_offset);
    } else {
        int x_offset = sidebar_common_get_x_offset_expanded();
        draw_buttons_expanded(x_offset);
        draw_overlay_text(x_offset + 4);

        if (get_game_engine() == ENGINE_ENV_C3) {
            widget_minimap_draw(x_offset + 8, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 0);
            draw_number_of_messages(x_offset);
        } else if (get_game_engine() == ENGINE_ENV_PHARAOH) {
            widget_minimap_draw(x_offset + 12, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 0);
            draw_number_of_messages(x_offset - 26);
        }

    }
    sidebar_extra_draw_foreground();
}
void widget_sidebar_city_draw_foreground_military(void) {
    widget_minimap_draw(sidebar_common_get_x_offset_expanded() + 8, MINIMAP_Y_OFFSET, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);
}
int widget_sidebar_city_handle_mouse(const mouse *m) {
    if (widget_city_has_input())
        return 0;

    int handled = 0;
    int button_id;
    data.focus_button_for_tooltip = 0;
    if (city_view_is_sidebar_collapsed()) {
        int x_offset = sidebar_common_get_x_offset_collapsed();
        handled |= image_buttons_handle_mouse(m, x_offset, 24, button_expand_sidebar[get_game_engine()], 1, &button_id);
        if (button_id)
            data.focus_button_for_tooltip = 12;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_build_collapsed[get_game_engine()], 12, &button_id);
        if (button_id)
            data.focus_button_for_tooltip = button_id + 19;

    } else {
        if (widget_minimap_handle_mouse(m))
            return 1;

        int x_offset = sidebar_common_get_x_offset_expanded();
        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_overlays_collapse_sidebar[get_game_engine()], 2,
                                              &button_id);
        if (button_id)
            data.focus_button_for_tooltip = button_id + 9;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_build_expanded[get_game_engine()], 15, &button_id);
        if (button_id)
            data.focus_button_for_tooltip = button_id + 19;

        handled |= image_buttons_handle_mouse(m, x_offset, 24, buttons_top_expanded[get_game_engine()], 6, &button_id);
        if (button_id)
            data.focus_button_for_tooltip = button_id + 39;

        handled |= sidebar_extra_handle_mouse(m);
    }
//    return 0;
    return handled;
}
int widget_sidebar_city_handle_mouse_build_menu(const mouse *m) {
    if (city_view_is_sidebar_collapsed())
        return image_buttons_handle_mouse(m, sidebar_common_get_x_offset_collapsed(), 24,
                                          buttons_build_collapsed[get_game_engine()], 12, 0);
    else
        return image_buttons_handle_mouse(m, sidebar_common_get_x_offset_expanded(), 24,
                                          buttons_build_expanded[get_game_engine()], 15, 0);
}
int widget_sidebar_city_get_tooltip_text(void) {
    return data.focus_button_for_tooltip;
}

static void slide_finished(void) {
    city_view_toggle_sidebar();
    window_city_show();
    window_draw(1);
}

static void button_overlay(int param1, int param2) {
    window_overlay_menu_show();
}
static void button_collapse_expand(int param1, int param2) {
    city_view_start_sidebar_toggle();
    sidebar_slide(!city_view_is_sidebar_collapsed(), draw_collapsed_background, draw_expanded_background,
                  slide_finished);
}
static void button_build(int submenu, int param2) {
    window_build_menu_show(submenu);
}
static void button_undo(int param1, int param2) {
    game_undo_perform();
    window_invalidate();
}
static void button_messages(int param1, int param2) {
    window_message_list_show();
}
static void button_help(int param1, int param2) {
    window_message_dialog_show(param2, window_city_draw_all);
}
static void button_go_to_problem(int param1, int param2) {
    int grid_offset = city_message_next_problem_area_grid_offset();
    if (grid_offset) {
        city_view_go_to_grid_offset(grid_offset);
        window_city_show();
    } else {
        window_invalidate();
    }
}
static void button_advisors(int param1, int param2) {
    window_advisors_show_checked();
}
static void button_empire(int param1, int param2) {
    window_empire_show_checked();
}
static void button_mission_briefing(int param1, int param2) {
    if (!scenario_is_custom())
        window_mission_briefing_show_review();

}
static void button_rotate_north(int param1, int param2) {
    game_orientation_rotate_north();
    window_invalidate();
}
static void button_rotate(int clockwise, int param2) {
    if (clockwise)
        game_orientation_rotate_right();

    else {
        game_orientation_rotate_left();
    }
    window_invalidate();
}
