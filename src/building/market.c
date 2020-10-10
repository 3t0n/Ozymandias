#include "market.h"

#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

struct resource_data {
    int building_id;
    int distance;
    int num_buildings;
};

int building_market_get_max_food_stock(building *market) {
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock)
                max_stock = stock;

        }
    }
    return max_stock;
}

int building_market_get_max_goods_stock(building *market) {
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_GOOD; i < INVENTORY_MAX_GOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock)
                max_stock = stock;

        }
    }
    return max_stock;
}

static void update_food_resource(struct resource_data *data, int resource, const building *b, int distance) {
    if (b->data.granary.resource_stored[resource]) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

static void update_good_resource(struct resource_data *data, int resource, building *b, int distance) {
    if (!city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

int is_good_accepted(int resource, building *market) {
    int goods_bit = 1 << resource;
    return !(market->subtype.market_goods & goods_bit);
}

void toggle_good_accepted(int resource, building *market) {
    int goods_bit = 1 << resource;
    market->subtype.market_goods ^= goods_bit;
}

void unaccept_all_goods(building *market) {
    market->subtype.market_goods = 0xFFFF;
}

int building_market_get_storage_destination(building *market) {
    struct resource_data resources[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].num_buildings = 0;
        resources[i].distance = 40;
    }
    for (int i = 1; i < MAX_BUILDINGS[GAME_ENV]; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_VALID)
            continue;

        if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE)
            continue;

        if (!b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != market->road_network_id) {
            continue;
        }
        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_MARKET, b))
            continue;

        int distance = calc_maximum_distance(market->x, market->y, b->x, b->y);
        if (distance >= 40)
            continue;

        if (b->type == BUILDING_GRANARY) {
            if (scenario_property_rome_supplies_wheat())
                continue;
            if (GAME_ENV == ENGINE_ENV_C3) {
                update_food_resource(&resources[INVENTORY_FOOD1], RESOURCE_WHEAT, b, distance);
                update_food_resource(&resources[INVENTORY_FOOD2], RESOURCE_VEGETABLES, b, distance);
                update_food_resource(&resources[INVENTORY_FOOD3], RESOURCE_FRUIT, b, distance);
                update_food_resource(&resources[INVENTORY_FOOD4], RESOURCE_MEAT_C3, b, distance);
            } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
                // todo: fetch map available foods?
                update_food_resource(&resources[INVENTORY_FOOD1], ALLOWED_FOODS[0], b, distance);
                update_food_resource(&resources[INVENTORY_FOOD2], ALLOWED_FOODS[1], b, distance);
                update_food_resource(&resources[INVENTORY_FOOD3], ALLOWED_FOODS[2], b, distance);
                update_food_resource(&resources[INVENTORY_FOOD4], ALLOWED_FOODS[3], b, distance);
            }
        } else if (b->type == BUILDING_WAREHOUSE) {
            // goods
            if (GAME_ENV == ENGINE_ENV_C3) {
                update_good_resource(&resources[INVENTORY_GOOD1], RESOURCE_POTTERY_C3, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD2], RESOURCE_FURNITURE, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD3], RESOURCE_OIL_C3, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD4], RESOURCE_WINE, b, distance);
            } else if (GAME_ENV == ENGINE_ENV_PHARAOH) {
                update_good_resource(&resources[INVENTORY_GOOD1], RESOURCE_POTTERY_PH, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD2], RESOURCE_LUXURY_GOODS, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD3], RESOURCE_LINEN, b, distance);
                update_good_resource(&resources[INVENTORY_GOOD4], RESOURCE_BEER, b, distance);
            }
        }
    }

    // update demands
    if (market->data.market.pottery_demand)
        market->data.market.pottery_demand--;
    else {
        resources[INVENTORY_GOOD1].num_buildings = 0;
    }
    if (market->data.market.furniture_demand)
        market->data.market.furniture_demand--;
    else {
        resources[INVENTORY_GOOD2].num_buildings = 0;
    }
    if (market->data.market.oil_demand)
        market->data.market.oil_demand--;
    else {
        resources[INVENTORY_GOOD3].num_buildings = 0;
    }
    if (market->data.market.wine_demand)
        market->data.market.wine_demand--;
    else {
        resources[INVENTORY_GOOD4].num_buildings = 0;
    }

    int can_go = 0;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].num_buildings) {
            can_go = 1;
            break;
        }
    }
    if (!can_go)
        return 0;

    // prefer food if we don't have it
    if (!market->data.market.inventory[INVENTORY_FOOD1] && resources[INVENTORY_FOOD1].num_buildings &&
        is_good_accepted(INVENTORY_FOOD1, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FOOD1;
        return resources[INVENTORY_FOOD1].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FOOD2] && resources[INVENTORY_FOOD2].num_buildings &&
               is_good_accepted(INVENTORY_FOOD2, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FOOD2;
        return resources[INVENTORY_FOOD2].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FOOD3] && resources[INVENTORY_FOOD3].num_buildings &&
               is_good_accepted(INVENTORY_FOOD3, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FOOD3;
        return resources[INVENTORY_FOOD3].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FOOD4] && resources[INVENTORY_FOOD4].num_buildings &&
               is_good_accepted(INVENTORY_FOOD4, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FOOD4;
        return resources[INVENTORY_FOOD4].building_id;
    }
    // then prefer resource if we don't have it
    if (!market->data.market.inventory[INVENTORY_GOOD1] && resources[INVENTORY_GOOD1].num_buildings &&
        is_good_accepted(INVENTORY_GOOD1, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_GOOD1;
        return resources[INVENTORY_GOOD1].building_id;
    } else if (!market->data.market.inventory[INVENTORY_GOOD2] && resources[INVENTORY_GOOD2].num_buildings &&
               is_good_accepted(INVENTORY_GOOD2, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_GOOD2;
        return resources[INVENTORY_GOOD2].building_id;
    } else if (!market->data.market.inventory[INVENTORY_GOOD3] && resources[INVENTORY_GOOD3].num_buildings &&
               is_good_accepted(INVENTORY_GOOD3, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_GOOD3;
        return resources[INVENTORY_GOOD3].building_id;
    } else if (!market->data.market.inventory[INVENTORY_GOOD4] && resources[INVENTORY_GOOD4].num_buildings &&
               is_good_accepted(INVENTORY_GOOD4, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_GOOD4;
        return resources[INVENTORY_GOOD4].building_id;
    }
    // then prefer smallest stock below 50
    int min_stock = 50;
    int fetch_inventory = -1;
    if (resources[INVENTORY_FOOD1].num_buildings &&
        market->data.market.inventory[INVENTORY_FOOD1] < min_stock
        && is_good_accepted(INVENTORY_FOOD1, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FOOD1];
        fetch_inventory = INVENTORY_FOOD1;
    }
    if (resources[INVENTORY_FOOD2].num_buildings &&
        market->data.market.inventory[INVENTORY_FOOD2] < min_stock
        && is_good_accepted(INVENTORY_FOOD2, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FOOD2];
        fetch_inventory = INVENTORY_FOOD2;
    }
    if (resources[INVENTORY_FOOD3].num_buildings &&
        market->data.market.inventory[INVENTORY_FOOD3] < min_stock
        && is_good_accepted(INVENTORY_FOOD3, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FOOD3];
        fetch_inventory = INVENTORY_FOOD3;
    }
    if (resources[INVENTORY_FOOD4].num_buildings &&
        market->data.market.inventory[INVENTORY_FOOD4] < min_stock
        && is_good_accepted(INVENTORY_FOOD4, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FOOD4];
        fetch_inventory = INVENTORY_FOOD4;
    }
    if (resources[INVENTORY_GOOD1].num_buildings &&
        market->data.market.inventory[INVENTORY_GOOD1] < min_stock
        && is_good_accepted(INVENTORY_GOOD1, market)) {
        min_stock = market->data.market.inventory[INVENTORY_GOOD1];
        fetch_inventory = INVENTORY_GOOD1;
    }
    if (resources[INVENTORY_GOOD2].num_buildings &&
        market->data.market.inventory[INVENTORY_GOOD2] < min_stock
        && is_good_accepted(INVENTORY_GOOD2, market)) {
        min_stock = market->data.market.inventory[INVENTORY_GOOD2];
        fetch_inventory = INVENTORY_GOOD2;
    }
    if (resources[INVENTORY_GOOD3].num_buildings &&
        market->data.market.inventory[INVENTORY_GOOD3] < min_stock
        && is_good_accepted(INVENTORY_GOOD3, market)) {
        min_stock = market->data.market.inventory[INVENTORY_GOOD3];
        fetch_inventory = INVENTORY_GOOD3;
    }
    if (resources[INVENTORY_GOOD4].num_buildings &&
        market->data.market.inventory[INVENTORY_GOOD4] < min_stock
        && is_good_accepted(INVENTORY_GOOD4, market)) {
        fetch_inventory = INVENTORY_GOOD4;
    }

    if (fetch_inventory == -1) {
        // all items well stocked: pick food below threshold
        if (resources[INVENTORY_FOOD1].num_buildings &&
            market->data.market.inventory[INVENTORY_FOOD1] < 600
            && is_good_accepted(INVENTORY_FOOD1, market)) {
            fetch_inventory = INVENTORY_FOOD1;
        }
        if (resources[INVENTORY_FOOD2].num_buildings &&
            market->data.market.inventory[INVENTORY_FOOD2] < 400
            && is_good_accepted(INVENTORY_FOOD2, market)) {
            fetch_inventory = INVENTORY_FOOD2;
        }
        if (resources[INVENTORY_FOOD3].num_buildings &&
            market->data.market.inventory[INVENTORY_FOOD3] < 400
            && is_good_accepted(INVENTORY_FOOD3, market)) {
            fetch_inventory = INVENTORY_FOOD3;
        }
        if (resources[INVENTORY_FOOD4].num_buildings &&
            market->data.market.inventory[INVENTORY_FOOD4] < 400
            && is_good_accepted(INVENTORY_FOOD4, market)) {
            fetch_inventory = INVENTORY_FOOD4;
        }
    }
    if (fetch_inventory < 0)
        return 0;

    market->data.market.fetch_inventory_id = fetch_inventory;
    return resources[fetch_inventory].building_id;
}
