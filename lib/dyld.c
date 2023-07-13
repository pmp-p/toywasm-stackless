#define _DARWIN_C_SOURCE /* snprintf */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "dyld.h"
#include "dylink_type.h"
#include "fileio.h"
#include "list.h"
#include "load_context.h"
#include "module.h"
#include "type.h"
#include "util.h"
#include "xlog.h"

int dyld_load_object_from_file(struct dyld *d, const char *name);

static const struct name name_GOT_mem = NAME_FROM_CSTR_LITERAL("GOT.mem");
static const struct name name_GOT_func = NAME_FROM_CSTR_LITERAL("GOT.func");
static const struct name name_env = NAME_FROM_CSTR_LITERAL("env");
static const struct name name_table_base =
        NAME_FROM_CSTR_LITERAL("__table_base");
static const struct name name_memory_base =
        NAME_FROM_CSTR_LITERAL("__memory_base");
static const struct name name_stack_pointer =
        NAME_FROM_CSTR_LITERAL("__stack_pointer");

static bool
is_GOT_mem_import(const struct import *im)
{
        return !compare_name(&name_GOT_mem, &im->module_name);
}

static bool
is_GOT_func_import(const struct import *im)
{
        return !compare_name(&name_GOT_func, &im->module_name);
}

static bool
is_env_func_import(const struct import *im)
{
        return im->desc.type == IMPORT_FUNC &&
               !compare_name(&name_env, &im->module_name);
}

void
dyld_init(struct dyld *d)
{
        memset(d, 0, sizeof(*d));
        LIST_HEAD_INIT(&d->objs);
}

int
dyld_load_needed_objects(struct dyld *d)
{
        int ret = 0;
        struct dyld_object *obj;
        LIST_FOREACH(obj, &d->objs, q) {
                const struct dylink_needs *needs = &obj->module->dylink->needs;
                uint32_t i;
                for (i = 0; i < needs->count; i++) {
                        const struct name *name = &needs->names[i];
                        char filename[PATH_MAX];
                        snprintf(filename, sizeof(filename), "%.*s",
                                 CSTR(name));
                        ret = dyld_load_object_from_file(d, filename);
                        if (ret != 0) {
                                goto fail;
                        }
                }
        }
fail:
        return ret;
}

int
dyld_load_object_from_file(struct dyld *d, const char *name)
{
        struct dyld_object *obj;
        int ret;
        obj = zalloc(sizeof(*obj));
        if (obj == NULL) {
                ret = ENOMEM;
                goto fail;
        }
        ret = map_file(name, (void *)&obj->bin, &obj->binsz);
        if (ret != 0) {
                goto fail;
        }
        struct load_context lctx;
        load_context_init(&lctx);
        ret = module_create(&obj->module, obj->bin, obj->bin + obj->binsz,
                            &lctx);
        load_context_clear(&lctx);
        if (ret != 0) {
                goto fail;
        }
        if (obj->module->dylink == NULL) {
                xlog_error("module %s doesn't have dylink.0", name);
                ret = EINVAL;
                goto fail;
        }
        LIST_INSERT_TAIL(&d->objs, obj, q);
fail:
        return ret;
}

static uint32_t
align_up(uint32_t v, uint32_t palign)
{
        uint32_t align = 1 << palign;
        uint32_t mask = align - 1;
        return (v + mask) & ~mask;
}

int
dyld_allocate_memory(struct dyld *d)
{
        uint32_t stack_size = 16 * 1024; /* XXX TODO guess from main obj */

        d->memory_base += stack_size;

        struct dyld_object *obj;
        LIST_FOREACH(obj, &d->objs, q) {
                const struct dylink_mem_info *minfo =
                        &obj->module->dylink->mem_info;
                d->memory_base =
                        align_up(d->memory_base, minfo->memoryalignment);
                obj->memory_base = d->memory_base;
                d->memory_base += minfo->memorysize;
        }
        return 0;
}

int
dyld_allocate_table(struct dyld *d)
{
        d->table_base += 1; /* do not use the first one */

        struct dyld_object *obj;
        LIST_FOREACH(obj, &d->objs, q) {
                const struct dylink_mem_info *minfo =
                        &obj->module->dylink->mem_info;
                d->table_base = align_up(d->table_base, minfo->tablealignment);
                obj->table_base = d->table_base;
                d->table_base += minfo->tablesize;
        }
        return 0;
}

int
dyld_load_main_object_from_file(struct dyld *d, const char *name)
{
        int ret;
        ret = dyld_load_object_from_file(d, name);
        if (ret != 0) {
                goto fail;
        }
        ret = dyld_load_needed_objects(d);
        if (ret != 0) {
                goto fail;
        }
        ret = dyld_allocate_memory(d);
        if (ret != 0) {
                goto fail;
        }
        ret = dyld_allocate_table(d);
        if (ret != 0) {
                goto fail;
        }
fail:
        return ret;
}