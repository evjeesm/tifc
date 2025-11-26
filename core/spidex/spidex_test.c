#include "spidex.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>

#define RUN_TEST(s,t,c) do { if (!run_test(#t,s,t,c)) { exit(1); }} while(0)

typedef void (*setup_func_t) (spidex_t *const spidex);
typedef bool (*test_func_t) (spidex_t *const spidex);
typedef void (*cleanup_func_t) (spidex_t *const spidex);

static bool run_test(const char *const test_name, setup_func_t setup_func, test_func_t test_func, cleanup_func_t cleanup_func);
static void setup(spidex_t *spidex);
static void setup_quad_frame(spidex_t *spidex);
static void cleanup(spidex_t *spidex);

static ssize_t value_cmp(const void *const value, const void *const element, void *const param)
{
    UNUSED(param);
    const size_t *range_value = value;
    const size_t *element_value = element;

    return *range_value - *element_value;
}

static bool test_empty(spidex_t *const spidex)
{
    // CASE: EMPTY AFTER CREATION
    if (!spidex_is_empty(spidex)) return false;

    // CASE: NOT EMPTY AFTER ADD
    spidex_area_t area = {.start = {0, 0}, .end = {10, 10}};
    spidex_value_t value = NULL;
    spidex_add(spidex, &area, value);

    if (spidex_is_empty(spidex)) return false;

    // ALL CASES PASS
    return true;
}

static bool test_has_intersect_single_area(spidex_t *const spidex)
{
    // CASE: NOT EMPTY AFTER ADD
    spidex_area_t area = {.start = {0, 0}, .end = {10, 10}};
    spidex_value_t value = NULL;
    spidex_add(spidex, &area, value);

    // CASE: INTERSECT WITH SAME AREA
    if (!spidex_has_intersect(spidex, &area)) return false;

    // CASE: LEFT ADJACENT
    spidex_area_t left_adjacent = {.start = {-10, 0}, .end = {0, 10}};
    if (spidex_has_intersect(spidex, &left_adjacent)) return false;

    // CASE: RIGHT ADJASENT
    spidex_area_t right_adjacent = {.start = {10, 0}, .end = {20, 10}};
    if (spidex_has_intersect(spidex, &right_adjacent)) return false;

    // CASE: TOP ADJACENT
    spidex_area_t top_adjacent = {.start = {0, -10}, .end = {10, 0}};
    if (spidex_has_intersect(spidex, &top_adjacent)) return false;

    // CASE: BOT ADJACENT
    spidex_area_t bot_adjacent = {.start = {0, 10}, .end = {10, 20}};
    if (spidex_has_intersect(spidex, &bot_adjacent)) return false;

    // CASE: INTERSECT WITH LEFT ALIGNED SMALLER AREA
    spidex_area_t left_aligned_smaller = {.start = {0, 0}, .end = {5, 5}};
    if (!spidex_has_intersect(spidex, &left_aligned_smaller)) return false;

    // CASE: INTERSECT WITH INNER AREA
    spidex_area_t inner = {.start = {1, 1}, .end = {5, 5}};
    if (!spidex_has_intersect(spidex, &inner)) return false;

    // CASE: INTERSECT WITH RIGHT ALIGNED SMALLER AREA
    spidex_area_t right_aligned_smaller = {.start = {5, 5}, .end = {10, 10}};
    if (!spidex_has_intersect(spidex, &right_aligned_smaller)) return false;

    // CASE: INTERSECT WITH LEFT ALIGNED BIGGER AREA
    spidex_area_t left_aligned_bigger = {.start = {0, 0}, .end = {15, 15}};
    if (!spidex_has_intersect(spidex, &left_aligned_bigger)) return false;

    // CASE: INTERSECT WITH OUTTER AREA
    spidex_area_t outer = {.start = {-1, -1}, .end = {11, 11}};
    if (!spidex_has_intersect(spidex, &outer)) return false;

    // CASE: INTERSECT WITH RIGHT ALIGNED BIGGER AREA
    spidex_area_t right_aligned_bigger = {.start = {-5, -5}, .end = {10, 10}};
    if (!spidex_has_intersect(spidex, &right_aligned_bigger)) return false;

    // CASE: TOP LEFT AREA
    spidex_area_t top_left = {.start = {-5, -5}, .end = {5, 5}};
    if (!spidex_has_intersect(spidex, &top_left)) return false;

    // CASE: TOP RIGHT
    spidex_area_t top_right = {.start = {5, -5}, .end = {15, 5}};
    if (!spidex_has_intersect(spidex, &top_right)) return false;

    // CASE: BOT LEFT
    spidex_area_t bot_left = {.start = {-5, 5}, .end = {5, 15}};
    if (!spidex_has_intersect(spidex, &bot_left)) return false;

    // CASE: BOT RIGHT
    spidex_area_t bot_right = {.start = {5, 5}, .end = {15, 15}};
    if (!spidex_has_intersect(spidex, &bot_right)) return false;

    // ALL CASES PASS
    return true;
}

bool test_has_intersect_many_areas(spidex_t *const spidex)
{
    UNUSED(spidex);
    return true;
}

int main(void)
{
    RUN_TEST(setup, test_empty, cleanup);
    RUN_TEST(setup, test_has_intersect_single_area, cleanup);
    RUN_TEST(setup_quad_frame, test_has_intersect_many_areas, cleanup);

    return 0;
}

static void setup(spidex_t *spidex)
{
    spidex_init(spidex, value_cmp);
}

static void setup_quad_frame(spidex_t *spidex)
{
    setup(spidex);
    // ttttttttrr
    // ttttttttrr
    // ll......rr
    // ll......rr
    // ll......rr
    // ll......rr
    // ll......rr
    // ll......rr
    // llbbbbbbbb
    // llbbbbbbbb
    spidex_value_t value = 0;
    spidex_area_t top = {.start = {0, 0}, .end = {8, 2}};
    spidex_add(spidex, &top, value++);

    spidex_area_t right = {.start = {8, 0}, .end = {10, 8}};
    spidex_add(spidex, &right, value++);

    spidex_area_t bot = {.start = {2, 8}, .end = {10, 10}};
    spidex_add(spidex, &bot, value++);

    spidex_area_t left = {.start = {0, 2}, .end = {2, 10}};
    spidex_add(spidex, &left, value++);
}

static void cleanup(spidex_t *spidex)
{
    spidex_deinit(spidex);
}

static bool run_test(const char *const test_name, setup_func_t setup_func, test_func_t test_func, cleanup_func_t cleanup_func)
{
    spidex_t spidex;
    setup_func(&spidex);

    bool result = test_func(&spidex);
    if (!result) fprintf(stderr, "TEST %s FAILED!\n", test_name);

    cleanup_func(&spidex);
    return result;
}
