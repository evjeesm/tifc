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

static bool test_has_intersect(spidex_t *const spidex)
{
    // CASE: NOT EMPTY AFTER ADD
    spidex_area_t area = {.start = {0, 0}, .end = {10, 10}};
    spidex_value_t value = NULL;
    spidex_add(spidex, &area, value);

    spidex_area_t intersect = {.start = {0, 0}, .end = {5, 5}};
    if (!spidex_has_intersect(spidex, &intersect)) return false;

    // ALL CASES PASS
    return true;
}

int main(void)
{
    RUN_TEST(setup, test_empty, cleanup);
    RUN_TEST(setup, test_has_intersect, cleanup);

    return 0;
}

static void setup(spidex_t *spidex)
{
    spidex_init(spidex, value_cmp);
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
