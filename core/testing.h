#ifndef _TESTING_H_
#define _TESTING_H_

#define TEST_ESC "\x1b"
#define TEST_COLOR_RED TEST_ESC "[31m"
#define TEST_COLOR_CLEAR TEST_ESC "[0m"

#define RED(msg) TEST_COLOR_RED msg TEST_COLOR_CLEAR

#define RUN_TEST(s,t,c) do { if (!run_test(#t,s,t,c)) { exit(1); }} while(0)
#define TEST_MSG(msg, ...) fprintf(stderr, msg "\n" ,##__VA_ARGS__)
#define TEST_ASSERT(cond, msg, ...) do { if (!(cond)) { TEST_MSG(msg,##__VA_ARGS__); return false; } } while (0)

#define TEST_SUCCESS() return true

#endif/*_TESTING_H_*/

