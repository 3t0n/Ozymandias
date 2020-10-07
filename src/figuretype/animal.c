#include "animal.h"

#include "building/building.h"
#include "city/entertainment.h"
#include "city/figures.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/point.h"
#include "scenario/map.h"
#include "scenario/property.h"

static const map_point SEAGULL_OFFSETS[] = {
        {0,  0},
        {0,  -2},
        {-2, 0},
        {1,  2},
        {2,  0},
        {-3, 1},
        {4,  -3},
        {-2, 4},
        {0,  0}
};

static const map_point HORSE_DESTINATION_1[] = {
        {2,  1},
        {3,  1},
        {4,  1},
        {5,  1},
        {6,  1},
        {7,  1},
        {8,  1},
        {9,  1},
        {10, 1},
        {11, 1},
        {12, 2},
        {12, 3},
        {11, 3},
        {10, 3},
        {9,  3},
        {8,  3},
        {7,  3},
        {6,  3},
        {5,  3},
        {4,  3},
        {3,  3},
        {2,  2}
};
static const map_point HORSE_DESTINATION_2[] = {
        {12, 3},
        {11, 3},
        {10, 3},
        {9,  3},
        {8,  3},
        {7,  3},
        {6,  3},
        {5,  3},
        {4,  3},
        {3,  3},
        {2,  2},
        {2,  1},
        {3,  1},
        {4,  1},
        {5,  1},
        {6,  1},
        {7,  1},
        {8,  1},
        {9,  1},
        {10, 1},
        {11, 1},
        {12, 2}
};

static const int SHEEP_IMAGE_OFFSETS[] = {
        0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 4, 4, 5, 5, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5
};

enum {
    HORSE_CREATED = 0,
    HORSE_RACING = 1,
    HORSE_FINISHED = 2
};

static void create_fishing_point(int x, int y) {
    random_generate_next();
    figure *fish = figure_create(FIGURE_FISH_GULLS, x, y, DIR_0_TOP_RIGHT);
    fish->anim_frame = random_byte() & 0x1f;
    fish->progress_on_tile = random_byte() & 7;
    fish->set_cross_country_direction(fish->cross_country_x, fish->cross_country_y, 15 * fish->destination_x, 15 * fish->destination_y, 0);
}
void figure_create_fishing_points(void) {
    scenario_map_foreach_fishing_point(create_fishing_point);
}
static void create_herd(int x, int y) {
    int herd_type;
    int num_animals;
    switch (scenario_property_climate()) {
        case CLIMATE_CENTRAL:
            herd_type = FIGURE_SHEEP;
            num_animals = 10;
            break;
        case CLIMATE_NORTHERN:
            herd_type = FIGURE_WOLF;
            num_animals = 8;
            break;
        case CLIMATE_DESERT:
            herd_type = FIGURE_ZEBRA;
            num_animals = 12;
            break;
        default:
            return;
    }
    int formation_id = formation_create_herd(herd_type, x, y, num_animals);
    if (formation_id > 0) {
        for (int fig = 0; fig < num_animals; fig++) {
            random_generate_next();
            figure *f = figure_create(herd_type, x, y, DIR_0_TOP_RIGHT);
            f->action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
            f->formation_id = formation_id;
            f->wait_ticks = f->id & 0x1f;
        }
    }
}
void figure_create_herds(void) {
    scenario_map_foreach_herd_point(create_herd);
}

void figure::seagulls_action() {
    terrain_usage = TERRAIN_USAGE_ANY;
    is_ghost = 0;
    use_cross_country = 1;
    if (!(anim_frame & 3) && move_ticks_cross_country(1)) {
        progress_on_tile++;
        if (progress_on_tile > 8)
            progress_on_tile = 0;
        set_cross_country_destination(source_x + SEAGULL_OFFSETS[progress_on_tile].x, source_y + SEAGULL_OFFSETS[progress_on_tile].y);
    }
    if (id & 1) {
        image_set_animation(GROUP_FIGURE_SEAGULLS, 0, 54, 3);
//        figure_image_increase_offset(54);
//        sprite_image_id = image_id_from_group(GROUP_FIGURE_SEAGULLS) + anim_frame / 3;
    } else {
        image_set_animation(GROUP_FIGURE_SEAGULLS, 18, 72, 3);
//        figure_image_increase_offset(72);
//        sprite_image_id = image_id_from_group(GROUP_FIGURE_SEAGULLS) + 18 + anim_frame / 3;
    }
}
void figure::sheep_action() {
    const formation *m = formation_get(formation_id);
//    terrain_usage = TERRAIN_USAGE_ANIMAL;
//    use_cross_country = 0;
//    is_ghost = 0;
    city_figures_add_animal();
//    figure_image_increase_offset(6);

    switch (action_state) {
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            wait_ticks++;
            if (wait_ticks > 400) {
                wait_ticks = id & 0x1f;
                action_state = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                destination_x = m->destination_x + formation_layout_position_x(FORMATION_HERD, index_in_formation);
                destination_y = m->destination_y + formation_layout_position_y(FORMATION_HERD, index_in_formation);
                roam_length = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            move_ticks(1);
            if (direction == DIR_FIGURE_AT_DESTINATION || direction == DIR_FIGURE_LOST) {
                direction = previous_tile_direction;
                action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                wait_ticks = id & 0x1f;
            } else if (direction == DIR_FIGURE_REROUTE)
                route_remove();

            break;
    }
    int dir = figure_image_direction();
    if (action_state == FIGURE_ACTION_149_CORPSE) {
        sprite_image_id = image_id_from_group(GROUP_FIGURE_SHEEP) + 104 + figure_image_corpse_offset();
    } else if (action_state == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST) {
        if (id & 3)
            sprite_image_id = image_id_from_group(GROUP_FIGURE_SHEEP) + 48 + dir + 8 * SHEEP_IMAGE_OFFSETS[wait_ticks & 0x3f];
        else
            sprite_image_id = image_id_from_group(GROUP_FIGURE_SHEEP) + 96 + dir;
    } else
        image_set_animation(GROUP_FIGURE_SHEEP, 0, 6);
//        sprite_image_id = image_id_from_group(GROUP_FIGURE_SHEEP) + dir + 8 * anim_frame;
}
void figure::wolf_action() {
    const formation *m = formation_get(formation_id);
//    terrain_usage = TERRAIN_USAGE_ANIMAL;
//    use_cross_country = 0;
//    is_ghost = 0;
    city_figures_add_animal();
//    figure_image_increase_offset(12);

    switch (action_state) {
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            wait_ticks++;
            if (wait_ticks > 400) {
                wait_ticks = id & 0x1f;
                action_state = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                destination_x = m->destination_x + formation_layout_position_x(FORMATION_HERD, index_in_formation);
                destination_y = m->destination_y + formation_layout_position_y(FORMATION_HERD, index_in_formation);
                roam_length = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            move_ticks(2);
            if (direction == DIR_FIGURE_AT_DESTINATION || direction == DIR_FIGURE_LOST) {
                direction = previous_tile_direction;
                action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                wait_ticks = id & 0x1f;
            } else if (direction == DIR_FIGURE_REROUTE)
                route_remove();
            break;
        case FIGURE_ACTION_199_WOLF_ATTACKING:
            move_ticks(2);
            if (direction == DIR_FIGURE_AT_DESTINATION) {
                int target_id = figure_combat_get_target_for_wolf(tile_x, tile_y, 6);
                if (target_id) {
                    figure *target = figure_get(target_id);
                    destination_x = target->tile_x;
                    destination_y = target->tile_y;
                    target_figure_id = target_id;
                    target->targeted_by_figure_id = id;
                    target_figure_created_sequence = target->created_sequence;
                    route_remove();
                } else {
                    direction = previous_tile_direction;
                    action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                    wait_ticks = id & 0x1f;
                }
            } else if (direction == DIR_FIGURE_REROUTE)
                route_remove();
            else if (direction == DIR_FIGURE_LOST) {
                direction = previous_tile_direction;
                action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                wait_ticks = id & 0x1f;
            }
            break;
    }
    int dir = figure_image_direction();
    if (action_state == FIGURE_ACTION_149_CORPSE)
        sprite_image_id = image_id_from_group(GROUP_FIGURE_WOLF) + 96 + figure_image_corpse_offset();
    else if (action_state == FIGURE_ACTION_150_ATTACK)
        sprite_image_id = image_id_from_group(GROUP_FIGURE_WOLF) + 104 + dir + 8 * (attack_image_offset / 4);
    else if (action_state == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST)
        sprite_image_id = image_id_from_group(GROUP_FIGURE_WOLF) + 152 + dir;
    else
        sprite_image_id = image_id_from_group(GROUP_FIGURE_WOLF) + dir + 8 * anim_frame;
}
void figure::zebra_action() {
    const formation *m = formation_get(formation_id);
//    terrain_usage = TERRAIN_USAGE_ANIMAL;
//    use_cross_country = 0;
//    is_ghost = 0;
    city_figures_add_animal();
//    figure_image_increase_offset(12);

    switch (action_state) {
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            wait_ticks++;
            if (wait_ticks > 200) {
                wait_ticks = id & 0x1f;
                action_state = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                destination_x = m->destination_x + formation_layout_position_x(FORMATION_HERD, index_in_formation);
                destination_y = m->destination_y + formation_layout_position_y(FORMATION_HERD, index_in_formation);
                roam_length = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            move_ticks(2);
            if (direction == DIR_FIGURE_AT_DESTINATION || direction == DIR_FIGURE_LOST) {
                direction = previous_tile_direction;
                action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                wait_ticks = id & 0x1f;
            } else if (direction == DIR_FIGURE_REROUTE)
                route_remove();

            break;
    }
    int dir = figure_image_direction();
    if (action_state == FIGURE_ACTION_149_CORPSE)
        sprite_image_id = image_id_from_group(GROUP_FIGURE_ZEBRA) + 96 + figure_image_corpse_offset();
    else if (action_state == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST)
        sprite_image_id = image_id_from_group(GROUP_FIGURE_ZEBRA) + dir;
    else {
        sprite_image_id = image_id_from_group(GROUP_FIGURE_ZEBRA) + dir + 8 * anim_frame;
    }
}

static void set_horse_destination(int state) {
//    building *b = building_get(building_id);
//    int orientation = city_view_orientation();
//    int rotation = b->subtype.orientation;
//    if (state == HORSE_CREATED) {
//        map_f->map_figure_remove();
//        if (rotation == 0) {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_6_TOP_LEFT) {
//                f->destination_x = b->x + HORSE_DESTINATION_1[f->wait_ticks_missile].x;
//                f->destination_y = b->y + HORSE_DESTINATION_1[f->wait_ticks_missile].y;
//            } else {
//                f->destination_x = b->x + HORSE_DESTINATION_2[f->wait_ticks_missile].x;
//                f->destination_y = b->y + HORSE_DESTINATION_2[f->wait_ticks_missile].y;
//            }
//        } else {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_2_BOTTOM_RIGHT) {
//                f->destination_x = b->x + HORSE_DESTINATION_1[f->wait_ticks_missile].y;
//                f->destination_y = b->y + HORSE_DESTINATION_1[f->wait_ticks_missile].x;
//            } else {
//                f->destination_x = b->x + HORSE_DESTINATION_2[f->wait_ticks_missile].y;
//                f->destination_y = b->y + HORSE_DESTINATION_2[f->wait_ticks_missile].x;
//            }
//        }
//        if (f->resource_id == 1)
//            f->destination_y++;
//
//        f->x = f->destination_x;
//        f->y = f->destination_y;
//        f->cross_country_x = 15 * f->x;
//        f->cross_country_y = 15 * f->y;
//        f->grid_offset_figure = map_grid_offset(f->x, f->y);
//        map_figure_add();
//    } else if (state == HORSE_RACING) {
//        if (rotation == 0) {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_6_TOP_LEFT) {
//                f->destination_x = b->x + HORSE_DESTINATION_1[f->wait_ticks_missile].x;
//                f->destination_y = b->y + HORSE_DESTINATION_1[f->wait_ticks_missile].y;
//            } else {
//                f->destination_x = b->x + HORSE_DESTINATION_2[f->wait_ticks_missile].x;
//                f->destination_y = b->y + HORSE_DESTINATION_2[f->wait_ticks_missile].y;
//            }
//        } else {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_2_BOTTOM_RIGHT) {
//                f->destination_x = b->x + HORSE_DESTINATION_1[f->wait_ticks_missile].y;
//                f->destination_y = b->y + HORSE_DESTINATION_1[f->wait_ticks_missile].x;
//            } else {
//                f->destination_x = b->x + HORSE_DESTINATION_2[f->wait_ticks_missile].y;
//                f->destination_y = b->y + HORSE_DESTINATION_2[f->wait_ticks_missile].x;
//            }
//        }
//    } else if (state == HORSE_FINISHED) {
//        if (rotation == 0) {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_6_TOP_LEFT) {
//                if (f->resource_id) {
//                    f->destination_x = b->x + 1;
//                    f->destination_y = b->y + 2;
//                } else {
//                    f->destination_x = b->x + 1;
//                    f->destination_y = b->y + 1;
//                }
//            } else {
//                if (f->resource_id) {
//                    f->destination_x = b->x + 12;
//                    f->destination_y = b->y + 3;
//                } else {
//                    f->destination_x = b->x + 12;
//                    f->destination_y = b->y + 2;
//                }
//            }
//        } else {
//            if (orientation == DIR_0_TOP_RIGHT || orientation == DIR_2_BOTTOM_RIGHT) {
//                if (f->resource_id) {
//                    f->destination_x = b->x + 2;
//                    f->destination_y = b->y + 1;
//                } else {
//                    f->destination_x = b->x + 1;
//                    f->destination_y = b->y + 1;
//                }
//            } else {
//                if (f->resource_id) {
//                    f->destination_x = b->x + 3;
//                    f->destination_y = b->y + 12;
//                } else {
//                    f->destination_x = b->x + 2;
//                    f->destination_y = b->y + 12;
//                }
//            }
//        }
//    }
}
void figure::hippodrome_horse_action() {
//    city_entertainment_set_hippodrome_has_race(1);
//    f->use_cross_country = 1;
//    f->is_ghost = 0;
//    figure_image_increase_offset(8);
//    if (!(building_get(building_id)->state)) {
//        f->kill();
//        return;
//    }
//    switch (f->action_state) {
//        case FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED:
//            f->anim_frame = 0;
//            f->wait_ticks_missile = 0;
//            set_horse_destination(f, HORSE_CREATED);
//            f->wait_ticks++;
//            if (f->wait_ticks > 60 && f->resource_id == 0) {
//                f->action_state = FIGURE_ACTION_201_HIPPODROME_HORSE_RACING;
//                f->wait_ticks = 0;
//            }
//            f->wait_ticks++;
//            if (f->wait_ticks > 20 && f->resource_id == 1) {
//                f->action_state = FIGURE_ACTION_201_HIPPODROME_HORSE_RACING;
//                f->wait_ticks = 0;
//            }
//            break;
//        case FIGURE_ACTION_201_HIPPODROME_HORSE_RACING:
//            f->direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
//            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
//                f->wait_ticks_missile++;
//                if (f->wait_ticks_missile >= 22) {
//                    f->wait_ticks_missile = 0;
//                    f->leading_figure_id++;
//                    if (f->leading_figure_id >= 6) {
//                        f->wait_ticks = 0;
//                        f->action_state = FIGURE_ACTION_202_HIPPODROME_HORSE_DONE;
//                    }
//                    if ((f->id + random_byte()) & 1)
//                        f->speed_multiplier = 3;
//                    else {
//                        f->speed_multiplier = 4;
//                    }
//                } else if (f->wait_ticks_missile == 11) {
//                    if ((f->id + random_byte()) & 1)
//                        f->speed_multiplier = 3;
//                    else {
//                        f->speed_multiplier = 4;
//                    }
//                }
//                set_horse_destination(f, HORSE_RACING);
//                f->direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
//                figure_movement_set_cross_country_direction(f,
//                                                            f->cross_country_x, f->cross_country_y,
//                                                            15 * f->destination_x, 15 * f->destination_y, 0);
//            }
//            if (f->action_state != FIGURE_ACTION_202_HIPPODROME_HORSE_DONE)
//                move_ticks_cross_country(f->speed_multiplier);
//
//            break;
//        case FIGURE_ACTION_202_HIPPODROME_HORSE_DONE:
//            if (!f->wait_ticks) {
//                set_horse_destination(f, HORSE_FINISHED);
//                f->direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
//                figure_movement_set_cross_country_direction(f,
//                                                            f->cross_country_x, f->cross_country_y,
//                                                            15 * f->destination_x, 15 * f->destination_y, 0);
//            }
//            if (f->direction != DIR_FIGURE_AT_DESTINATION)
//                move_ticks_cross_country(1);
//
//            f->wait_ticks++;
//            if (f->wait_ticks > 30)
//                f->anim_frame = 0;
//
//            f->wait_ticks++;
//            if (f->wait_ticks > 150)
//                f->kill();
//
//            break;
//    }
//
//    int dir = figure_image_direction(f);
//    if (f->resource_id == 0) {
//        f->sprite_image_id = image_id_from_group(GROUP_FIGURE_HIPPODROME_HORSE_1) +
//                      dir + 8 * f->anim_frame;
//        f->cart_image_id = image_id_from_group(GROUP_FIGURE_HIPPODROME_CART_1) + dir;
//    } else {
//        f->sprite_image_id = image_id_from_group(GROUP_FIGURE_HIPPODROME_HORSE_2) +
//                      dir + 8 * f->anim_frame;
//        f->cart_image_id = image_id_from_group(GROUP_FIGURE_HIPPODROME_CART_2) + dir;
//    }
//    int cart_dir = (dir + 4) % 8;
//    figure_image_set_cart_offset(f, cart_dir);
}
void figure_hippodrome_horse_reroute(void) {
    if (!city_entertainment_hippodrome_has_race())
        return;
    for (int i = 1; i < MAX_FIGURES[GAME_ENV]; i++) {
        figure *f = figure_get(i);
        if (f->state == FIGURE_STATE_ALIVE && f->type == FIGURE_HIPPODROME_HORSES) {
            f->wait_ticks_missile = 0;
            set_horse_destination(HORSE_CREATED);
        }
    }
}
