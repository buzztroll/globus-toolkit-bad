
/**
 * @file globus_symboltable.h
 * @brief Lightweight Chaining Symboltable
 */

#ifndef GLOBUS_SYMBOLTABLE_H
#define GLOBUS_SYMBOLTABLE_H

#include "globus_hashtable.h"

#ifdef __cplusplus
extern "C" {
#endif

struct globus_symboltable_s;
typedef struct globus_symboltable_s *               globus_symboltable_t;

extern int 
globus_symboltable_init(
    globus_symboltable_t          *                 table,
    globus_hashtable_hash_func_t                    hash_func,
    globus_hashtable_keyeq_func_t                   keyeq_func);


extern void *
globus_symboltable_lookup (globus_symboltable_t *table, void *symbol);

extern int 
globus_symboltable_insert (globus_symboltable_t *table, 
                           void *symbol, 
                           void *datum);

extern void *
globus_symboltable_remove (globus_symboltable_t *table, void *symbol);


extern int
globus_symboltable_create_scope (globus_symboltable_t *table);

extern int
globus_symboltable_remove_scope (globus_symboltable_t *table);


extern int 
globus_symboltable_destroy (globus_symboltable_t *table);


#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_SYMBOLTABLE_H */
