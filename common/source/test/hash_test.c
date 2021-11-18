
/**
 * @file hash_test.c
 * @brief Hashtable Test Cases
 */

#include "globus_common.h"
#include "globus_test_tap.h"

/** @brief Globus Hashtable Test Cases */
int hash_test(void)
{
    globus_hashtable_t hash_table;

    printf("1..8\n");
    /**
     * @test
     * Initialize hashtable with globus_hashtable_init()
     */
    ok(globus_hashtable_init(&hash_table,
                          256,
                          globus_hashtable_int_hash,
                          globus_hashtable_int_keyeq) == 0, "hashtable_init");

    /**
     * @test
     * Insert datum 123 into hashtable with globus_hashtable_insert()
     */
    ok(globus_hashtable_insert(&hash_table,
                            (void *) 123,
                            (void *) "xyz") == 0, "insert_123");
    /**
     * @test
     * Insert datum 456 into hashtable with globus_hashtable_insert()
     */
    ok(globus_hashtable_insert(&hash_table,
                            (void *) 456,
                            (void *) "abc") == 0, "insert_456");
    /**
     * @test
     * Insert datum 111 into hashtable with globus_hashtable_insert()
     */
    ok(globus_hashtable_insert(&hash_table,
                            (void *) 111,
                            (void *) "aaa") == 0, "insert_111");
    /**
     * @test
     * Insert datum 222 into hashtable with globus_hashtable_insert()
     */
    ok(globus_hashtable_insert(&hash_table,
                            (void *) 222,
                            (void *) "bbb") == 0, "insert_222");

    /**
     * @test
     * Remove datum 222 from hashtable with globus_hashtable_remove()
     */
    ok(strcmp(globus_hashtable_remove(&hash_table,
                                   (void *) 222), "bbb") == 0, "remove_222");
    /**
     * @test
     * Remove datum 456 from hashtable with globus_hashtable_remove()
     */
    ok(strcmp(globus_hashtable_remove(&hash_table,
                                   (void *) 456), "abc") == 0, "remove_456");

    /**
     * @test
     * Destroy hashtable with globus_hashtable_destroy()
     */
    ok(globus_hashtable_destroy(&hash_table) == 0, "hashtable_destroy");
    return TEST_EXIT_CODE;
}

int main(int argc, char **argv)
{
    return hash_test();
}
