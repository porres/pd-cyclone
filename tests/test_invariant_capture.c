#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 
 * Security invariant: Buffer writes from sprintf must never exceed buffer bounds.
 * The capture~ object constructs format strings with user-controlled precision,
 * which can cause sprintf to overflow the destination buffer.
 * 
 * Since we cannot directly call the internal capture_dowrite function,
 * we test the vulnerable pattern: sprintf with precision-controlled format strings.
 */

#define CAPTURE_BUFSIZE 16384  /* From capture.c: CAPTURE_DEFSIZE */
#define CAPTURE_MAXPRECISION 16 /* Safe maximum precision for floats */

START_TEST(test_capture_precision_overflow)
{
    /* Invariant: sprintf output with precision must not exceed buffer size */
    int precision_values[] = {
        1000,   /* Exploit case: causes massive overflow */
        100,    /* Boundary: still dangerous */
        6       /* Valid: default precision */
    };
    int num_tests = sizeof(precision_values) / sizeof(precision_values[0]);
    
    char format[32];
    char small_buffer[64];  /* Simulates undersized buffer scenario */
    float test_value = 3.14159265358979323846f;
    
    for (int i = 0; i < num_tests; i++) {
        int precision = precision_values[i];
        
        /* This is how capture.c builds format strings (line 365) */
        sprintf(format, "%%.%df\n", precision);
        
        /* Calculate required buffer size BEFORE writing */
        int required_size = snprintf(NULL, 0, format, test_value);
        
        /* Security check: precision must be bounded to prevent overflow */
        ck_assert_msg(precision <= CAPTURE_MAXPRECISION || required_size < (int)sizeof(small_buffer),
            "Precision %d would require %d bytes, exceeding safe buffer size",
            precision, required_size);
        
        /* If precision is safe, verify the write fits */
        if (precision <= CAPTURE_MAXPRECISION) {
            int written = snprintf(small_buffer, sizeof(small_buffer), format, test_value);
            ck_assert_int_lt(written, (int)sizeof(small_buffer));
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_capture_precision_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}