#ifndef HELPER_H
#define HELPER_H

#include "check.h"

#define TEST_MAIN int main (int argc, char *argv[]) { \
int number_failed; \
Suite *suite = build_suite(); \
SRunner *runner = srunner_create(suite); \
srunner_run_all(runner, CK_NORMAL); \
number_failed = srunner_ntests_failed(runner); \
srunner_free(runner); \
return number_failed; \
}


#endif /* HELPER_H */
