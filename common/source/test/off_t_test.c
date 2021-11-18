
/**
 * @file off_t_test.c
 * @brief Test of globus_off_t
 */

#include "globus_common.h"

/**
 * @brief globus_off_t size test
 */
int
globus_off_t_test(void)
{
    int rc1, rc2;
    globus_off_t big;
    char buf1[100];
    char buf2[100];

    printf("1..2\n");
    
    /** 
     * @test
     * Check that globus_off_t is at least 64 bits long
     */
    rc1 = (sizeof(globus_off_t) < 8);
    printf("%sok 1 globus_off_t_at_least_64_bits\n", rc1 == 0 ? "" : "not ");

    /**
     * @test
     * Check that GLOBUS_OFF_T_FORMAT can handle a INT64_MAX valued
     * globus_off_t.
     */
    big = INT64_MAX;
    snprintf(buf1, sizeof(buf1), "%"PRId64, big);
    snprintf(buf2, sizeof(buf2), "%"GLOBUS_OFF_T_FORMAT, big);
    rc2 = strcmp(buf1, buf2);

    printf("%sok 2 globus_off_t_format_test\n", (rc2 == 0) ? "" : "not");

    return rc1||rc2;
}

int
main(int argc, char * argv[])
{
    return globus_off_t_test();
}
/* main() */
