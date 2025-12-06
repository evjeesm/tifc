#include "spidex.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>

#define TEST_ESC "\x1b"
#define TEST_COLOR_RED TEST_ESC "[31m"
#define TEST_COLOR_CLEAR TEST_ESC "[0m"

#define RED(msg) TEST_COLOR_RED msg TEST_COLOR_CLEAR

#define RUN_TEST(s,t,c) do { if (!run_test(#t,s,t,c)) { exit(1); }} while(0)
#define TEST_MSG(msg, ...) fprintf(stderr, msg "\n" ,##__VA_ARGS__)
#define TEST_ASSERT(cond, msg, ...) do { if (!(cond)) { TEST_MSG(msg,##__VA_ARGS__); return false; } } while (0)

#define TEST_SUCCESS() return true

typedef void (*setup_func_t) (spidex_t *const spidex);
typedef bool (*test_func_t) (spidex_t *const spidex);
typedef void (*cleanup_func_t) (spidex_t *const spidex);

static bool run_test(const char *const test_name, setup_func_t setup_func, test_func_t test_func, cleanup_func_t cleanup_func);
static void setup(spidex_t *spidex);
static void setup_quad_frame(spidex_t *spidex);
static void setup_for_query(spidex_t *spidex);
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
    TEST_ASSERT(spidex_is_empty(spidex), RED("Spidex should be empty after creation!"));

    // CASE: NOT EMPTY AFTER ADD
    spidex_area_t area = {.start = {0, 0}, .end = {10, 10}};
    spidex_value_t value = NULL;
    spidex_add(spidex, &area, value);

    TEST_ASSERT(!spidex_is_empty(spidex),
                RED("Spidex should NOT be empty after spidex_add()!"));

    TEST_SUCCESS();
}

static bool test_has_intersect_single_area(spidex_t *const spidex)
{
    // CASE: NOT EMPTY AFTER ADD
    spidex_area_t area = {.start = {0, 0}, .end = {10, 10}};
    spidex_value_t value = NULL;
    spidex_add(spidex, &area, value);

    // CASE: INTERSECT WITH SAME AREA
    TEST_ASSERT((spidex_has_intersect(spidex, &area)),
        RED("Intersect with area of same size should be detected!"));

    // CASE: LEFT ADJACENT
    spidex_area_t left_adjacent = {.start = {-10, 0}, .end = {0, 10}};
    TEST_ASSERT((!spidex_has_intersect(spidex, &left_adjacent)),
        RED("No intersection should be detected with adjacent area from the left!"));

    // CASE: RIGHT ADJASENT
    spidex_area_t right_adjacent = {.start = {10, 0}, .end = {20, 10}};
    TEST_ASSERT((!spidex_has_intersect(spidex, &right_adjacent)),
        RED("No intersection should be detected with adjacent area from the right!"));

    // CASE: TOP ADJACENT
    spidex_area_t top_adjacent = {.start = {0, -10}, .end = {10, 0}};
    TEST_ASSERT((!spidex_has_intersect(spidex, &top_adjacent)),
        RED("No intersection should be detected with adjacent area from the top!"));

    // CASE: BOT ADJACENT
    spidex_area_t bot_adjacent = {.start = {0, 10}, .end = {10, 20}};
    TEST_ASSERT((!spidex_has_intersect(spidex, &bot_adjacent)),
        RED("No intersection should be detected with adjacent area from the bottom!"));

    // CASE: INTERSECT WITH LEFT ALIGNED SMALLER AREA
    spidex_area_t left_aligned_smaller = {.start = {0, 0}, .end = {5, 5}};
    TEST_ASSERT((spidex_has_intersect(spidex, &left_aligned_smaller)),
        RED("Intersection should be detected with left aligned area of smaller size!"));

    // CASE: INTERSECT WITH INNER AREA
    spidex_area_t inner = {.start = {1, 1}, .end = {5, 5}};
    TEST_ASSERT((spidex_has_intersect(spidex, &inner)),
        RED("Intersection should be detected with inner area!"));

    // CASE: INTERSECT WITH RIGHT ALIGNED SMALLER AREA
    spidex_area_t right_aligned_smaller = {.start = {5, 5}, .end = {10, 10}};
    TEST_ASSERT((spidex_has_intersect(spidex, &right_aligned_smaller)),
        RED("Intersection should be detected with right aligned area of smaller size!"));

    // CASE: INTERSECT WITH LEFT ALIGNED BIGGER AREA
    spidex_area_t left_aligned_bigger = {.start = {0, 0}, .end = {15, 15}};
    TEST_ASSERT((spidex_has_intersect(spidex, &left_aligned_bigger)),
        RED("Intersection should be detected with left aligned area of bigger size!"));

    // CASE: INTERSECT WITH OUTTER AREA
    spidex_area_t outer = {.start = {-1, -1}, .end = {11, 11}};
    TEST_ASSERT((spidex_has_intersect(spidex, &outer)),
        RED("Intersection should be detected with outer area!"));

    // CASE: INTERSECT WITH RIGHT ALIGNED BIGGER AREA
    spidex_area_t right_aligned_bigger = {.start = {-5, -5}, .end = {10, 10}};
    TEST_ASSERT((spidex_has_intersect(spidex, &right_aligned_bigger)),
        RED("Intersection should be detected with right aligned area of bigger size!"));

    // CASE: TOP LEFT AREA
    spidex_area_t top_left = {.start = {-5, -5}, .end = {5, 5}};
    TEST_ASSERT((spidex_has_intersect(spidex, &top_left)),
        RED("Intersection should be detected with top left shifted area!"));

    // CASE: TOP RIGHT
    spidex_area_t top_right = {.start = {5, -5}, .end = {15, 5}};
    TEST_ASSERT((spidex_has_intersect(spidex, &top_right)),
        RED("Intersection should be detected with top right shifted area!"));

    // CASE: BOT LEFT
    spidex_area_t bot_left = {.start = {-5, 5}, .end = {5, 15}};
    TEST_ASSERT((spidex_has_intersect(spidex, &bot_left)),
        RED("Intersection should be detected with bottom left shifted area!"));

    // CASE: BOT RIGHT
    spidex_area_t bot_right = {.start = {5, 5}, .end = {15, 15}};
    TEST_ASSERT((spidex_has_intersect(spidex, &bot_right)),
        RED("Intersection should be detected with bottom right shifted area!"));

    TEST_SUCCESS();
}

bool test_has_intersect_cutout_middle(spidex_t *const spidex)
{
    spidex_area_t middle = { .start = {2, 2}, .end = {8, 8} };

    TEST_ASSERT((!spidex_has_intersect(spidex, &middle)),
        RED("Intersect should not be detected with middle cut-out area!"));

    TEST_SUCCESS();
}

bool test_add(spidex_t *const spidex)
{
    spidex_area_t area = {{0,0}, {1,1}};
    spidex_value_t value = NULL;
    spidex_status_t status = spidex_add(spidex, &area, value);

    TEST_ASSERT((SPIDEX_OK == status),
        RED("Trivial add into an empty spidex should not fail"));

    TEST_SUCCESS();
}

bool test_add_sharing_x(spidex_t *const spidex)
{
    spidex_area_t area_a = {{5,5}, {6,6}}; // point at 5, 5
    spidex_area_t area_b = {{5,6}, {6,7}}; // point at 5, 6

    spidex_status_t status = spidex_add(spidex, &area_a, (spidex_value_t) 0xa);

    TEST_ASSERT((SPIDEX_OK == status),
        RED("Trivial add into an empty spidex should not fail"));

    status = spidex_add(spidex, &area_b, (spidex_value_t) 0xb);

    TEST_ASSERT((SPIDEX_OK == status),
        RED("Add of non-overlaping regions should not fail"));

    spidex_value_t value;

    status = spidex_query(spidex, area_a.start, &value);

    TEST_ASSERT((SPIDEX_OK == status),
        RED("query area_a after inserting area_b should return value successfully"));

    TEST_ASSERT(((spidex_value_t)0xa == value),
        RED("area_a value should be queried correctly"));

    status = spidex_query(spidex, area_b.start, &value);

    TEST_ASSERT((SPIDEX_OK == status),
        RED("query area_b should return value successfully"));

    TEST_ASSERT(((spidex_value_t)0xb == value),
        RED("area_b value should be queried correctly"));

    TEST_SUCCESS();
}

#define ARR_LEN(array) (sizeof(array)/sizeof(*array))

bool test_query(spidex_t *const spidex)
{
    spidex_pos_t p[] = {{ 4, 1 },   // f
                        { 6, 2 },   // a
                        { 7, 4 },   // c
                        { 6, 3 },}; // none

    const spidex_status_t exp_status[ARR_LEN(p)] = {
        SPIDEX_OK,
        SPIDEX_OK,
        SPIDEX_OK,
        SPIDEX_NO_VALUE
    };

    const spidex_value_t exp_values[ARR_LEN(p)] = {
        (spidex_value_t) 0xf,
        (spidex_value_t) 0xa,
        (spidex_value_t) 0xc,
        (spidex_value_t) 0x0, // don't care
    };

    for (size_t i = 0; i < ARR_LEN(p); ++i)
    {
        spidex_value_t value;
        spidex_status_t status = spidex_query(spidex, p[i], &value);
        TEST_ASSERT((exp_status[i] == status), "at p[%zu] Unexpected status (%d)", i, status);
        TEST_ASSERT((exp_values[i] == 0x0 || exp_values[i] == value), "at p[%zu] Unexpected value (%p)", i, value);
    }

    TEST_SUCCESS();
}


int main(void)
{
    RUN_TEST(setup, test_empty, cleanup);
    RUN_TEST(setup, test_add, cleanup);
    RUN_TEST(setup, test_add_sharing_x, cleanup);
    RUN_TEST(setup, test_has_intersect_single_area, cleanup);
    RUN_TEST(setup_quad_frame, test_has_intersect_cutout_middle, cleanup);
    RUN_TEST(setup_for_query, test_query, cleanup);

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

static void setup_for_query(spidex_t *spidex)
{
    setup(spidex);
    //...........
    //....ffbbe..
    //....aaaae..
    //.......cc..
    //.......cc..
    //...........
    spidex_area_t area_f = {.start = {4, 1}, .end = {6, 2}};
    spidex_add(spidex, &area_f, (spidex_value_t) 0xf);

    spidex_area_t area_b = {.start = {6, 1}, .end = {8, 2}};
    spidex_add(spidex, &area_b, (spidex_value_t) 0xb);

    spidex_area_t area_e = {.start = {8, 1}, .end = {9, 3}};
    spidex_add(spidex, &area_e, (spidex_value_t) 0xe);

    spidex_area_t area_a = {.start = {4, 2}, .end = {8, 3}};
    spidex_add(spidex, &area_a, (spidex_value_t) 0xa);

    spidex_area_t area_c = {.start = {7, 3}, .end = {9, 5}};
    spidex_add(spidex, &area_c, (spidex_value_t) 0xc);
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
    if (!result) TEST_MSG("["RED("FAILED")"] Test %s\n", test_name);


    cleanup_func(&spidex);
    return result;
}
