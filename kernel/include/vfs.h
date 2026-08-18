#pragma once

#include "sysresult.h"
#include <stdint.h>
#include <stdio.h>
#include <strslice.h>
#include <uuid.h>
#include <vtable.h>

typedef uint64_t object_t;
typedef uint32_t stream_t;

struct VfsData;

typedef struct VfsVtable {
    struct VtableCommon vcommon;
    uuid (*vfs_uuid)(struct VfsData *vfs_data);
    object_t (*vfs_root_obj)(struct VfsData *vfs_data);
    stream_t (*vfs_find_stream)(struct VfsData *vfs_data, object_t vfs_obj,
                                string_t vfs_stream_name);
    object_t (*vfs_search)(struct VfsData *vfs_data, object_t vfs_obj,
                           stream_t vfs_stream, string_t vfs_file_name);
    sysresult2_t (*vfs_read)(struct VfsData *vfs_data, object_t vfs_obj,
                             stream_t vfs_stream, void *vfs_rdata,
                             size_t vfs_size);
    sysresult2_t (*vfs_write)(struct VfsData *vfs_data, object_t vfs_obj,
                              stream_t vfs_stream, const void *vfs_rdata,
                              size_t vfs_size)
} vfs_vtable_t;

typedef struct {
    struct VsData *vfs_data;
    vfs_vtable_t *vfs_vptr;
} vfs_ptr_t;

typedef sysresult2_t vfs_virtual_provider_t(string_t vfs_name);
typedef sysresult2_t vfs_provider_t(string_t hint_name, FILE *backing);

void register_virtual_provider(vfs_virtual_provider_t *vfs_provider,
                               vfs_vtable_t *vfs_vtable, string_t vfs_name);

void register_data_provider(vfs_provider_t *vfs_provider,
                            vfs_vtable_t *vfs_vtable, string_t vfs_name);
