#define ROOT_DIR_ID  0xffffffffffffffff
#define END_OF_CHAIN 0xffffffffffffffff

#define ECHFS_TYPE_ANY  0xff
#define ECHFS_TYPE_FILE 0
#define ECHFS_TYPE_DIR  1

#define ECHFS_SEARCH_FAILURE 0xffffffffffffffff

#define ECHFS_BLKDEV_BLOCK_SIZE 512

#define ECHFS_BLKDEV_CACHE_NOTREADY 0
#define ECHFS_BLKDEV_CACHE_READY    1
#define ECHFS_BLKDEV_CACHE_DIRTY    2

Bool EchFSBlkDevReadBlocks(CDrive *drive, U8 *dest, U64 loc, U64 count) {
    U64 i;
    for (i = 0; i < count; i++)
        if (!BlkRead(drive, dest + i * 512, loc + i * 512, 1))
            return FALSE;
}

Bool EchFSBlkDevCache(CDrive *drive, U64 block) {
    switch (drive->echfs_id_table->dev_cache_state) {
        case ECHFS_BLKDEV_CACHE_READY:
            if (block == drive->echfs_id_table->dev_cached_block)
                return TRUE;
            // FALLTHRU
        case ECHFS_BLKDEV_CACHE_NOTREADY:
            //EchFSBlkDevReadBlocks(drive, drive->echfs_id_table->dev_cache,
            BlkRead(drive, drive->echfs_id_table->dev_cache,
                                  drive->drv_offset + block,
                                  ECHFS_BLKDEV_BLOCK_SIZE / 512);
            drive->echfs_id_table->dev_cached_block = block;
            drive->echfs_id_table->dev_cache_state  = ECHFS_BLKDEV_CACHE_READY;
            return TRUE;
        case ECHFS_BLKDEV_CACHE_DIRTY:
            "echfs cache flush\n";
            //while (1) {}
    }
}

Bool EchFSBlkDevRead(CDrive *drive, U8 *buffer, U64 loc, U64 count) {
    U64 block, chunk, offset, progress = 0;
    while (progress < count) {
        block = (loc + progress) / ECHFS_BLKDEV_BLOCK_SIZE;

        if (!EchFSBlkDevCache(drive, block))
            return FALSE;

        chunk = count - progress;
        offset = (loc + progress) % ECHFS_BLKDEV_BLOCK_SIZE;
        if (chunk > ECHFS_BLKDEV_BLOCK_SIZE - offset)
            chunk = ECHFS_BLKDEV_BLOCK_SIZE - offset;

        MemCopy(buffer + progress, &drive->echfs_id_table->dev_cache[offset], chunk);
        progress += chunk;
    }

    return TRUE;
}

Bool EchFSReadEntry(CDrive *drive, CEchFSEntry *ret, U64 entry) {
    CEchFSIdTable *id_tab = drive->echfs_id_table;
    U64 loc = id_tab->dir_start * id_tab->bytes_per_block +
              entry * sizeof(CEchFSEntry);

    if (loc >= (id_tab->dir_start + id_tab->dir_size) * id_tab->bytes_per_block)
        return FALSE;

    if (!EchFSBlkDevRead(drive, ret, loc, sizeof(CEchFSEntry)))
        return FALSE;

    return TRUE;
}

// Returns unique entry #, ECHFS_SEARCH_FAILURE upon failure/not found
U64 EchFSSearch(CDrive *drive, CEchFSEntry *entry, U8 *name, U64 parent, U8 type) {
    CEchFSIdTable *id_tab = drive->echfs_id_table;
    U64 i;

    for (i = 0; i < id_tab->dir_size * id_tab->dir_entries_per_block; i++) {
        if (!EchFSReadEntry(drive, entry, i))
            return ECHFS_SEARCH_FAILURE;

        if (!entry->parent_id)
            return ECHFS_SEARCH_FAILURE;

        if (entry->parent_id == parent &&
            (entry->type == type || type == ECHFS_TYPE_ANY) &&
            !StrCompare(entry->name, name))
            return i;
    }

    return ECHFS_SEARCH_FAILURE;
}

Bool EchFSResolvePath(CDrive *drive, CEchFSPathResult *ret, U8 *path, U8 type,
                      U64 parent) {
    U8          name[201];
    Bool        last = FALSE;
    U64         i, search_res;

    MemSet(ret, 0, sizeof(CEchFSPathResult));

    if (!StrCompare(path, "/")) {
        switch (type) {
            case ECHFS_TYPE_DIR:
                // If looking for a directory called "/", return root, as it does
                // not have an entry
                ret->target.type    = ECHFS_TYPE_DIR;
                ret->target.payload = ROOT_DIR_ID;
                ret->parent.type    = ECHFS_TYPE_DIR;
                ret->parent.payload = ROOT_DIR_ID;
                return TRUE;
            case ECHFS_TYPE_FILE:
                // Fail if looking for a file named "/"
                return FALSE;
        }
    }

    // Start with the parent
    ret->parent.payload = parent;

    if (*path == '/')
        path++;

next:
    for (i = 0; *path != '/'; path++) {
        if (!*path) {
            last = TRUE;
            break;
        }
        name[i++] = *path;
    }
    name[i] = 0;
    path++;

    if (!last) {
        search_res = EchFSSearch(drive, &ret->parent, name, ret->parent.payload,
                                     ECHFS_TYPE_DIR);
        if (search_res == ECHFS_SEARCH_FAILURE)
            return FALSE;
    } else {
        search_res = EchFSSearch(drive, &ret->target, name, ret->parent.payload,
                                     type);
        if (search_res == ECHFS_SEARCH_FAILURE)
            ret->not_found = TRUE;
        else
            ret->target_entry = search_res;
        return TRUE;
    }

    goto next;
}

Bool EchFSCd(U8 *name, I64 cur_dir_clus) {
    CEchFSPathResult pres;

    if (!EchFSResolvePath(Fs->cur_dv, &pres, name, ECHFS_TYPE_DIR, cur_dir_clus(U64)))
        return FALSE;

    if (pres.not_found)
        return FALSE;

    return TRUE;
}

Bool EchFSMkDir(CDrive *drive, U8 *cur_dir, U8 *name, I64 entry_count) {
	"echfs mkdir\n";
	//while (1) {}
}

U0 EchFSInit(CDrive *drive){
	"EchFSInit called\n";

	Bool unlock;

	try {
		unlock = DriveLock(drive);

		drive->echfs_id_table = MAlloc(sizeof(CEchFSIdTable));
		BlkRead(drive, drive->echfs_id_table, drive->drv_offset, 1);

		if (drive->echfs_id_table->signature != '_ECH_FS_') {
			"echfs: error: signature mismatch\n";
            //while (1) {}
		}
        "echfs: valid signature\n";

        "echfs: Block count: %u\n", drive->echfs_id_table->block_count;

        drive->echfs_id_table->fat_size =
            (drive->echfs_id_table->block_count * sizeof(U64) +
             drive->echfs_id_table->bytes_per_block - 1) /
            drive->echfs_id_table->bytes_per_block;
        "echfs: FAT size: %u\n", drive->echfs_id_table->fat_size;

        drive->echfs_id_table->fat_start = 16;
        "echfs: FAT start: %u\n", drive->echfs_id_table->fat_start;

        drive->echfs_id_table->dir_start = 16 + drive->echfs_id_table->fat_size;
        "echfs: Directory start: %u\n", drive->echfs_id_table->dir_start;

        drive->echfs_id_table->sectors_per_block =
            drive->echfs_id_table->bytes_per_block / 512;
        "echfs: Bytes per block: %u\n", drive->echfs_id_table->bytes_per_block;
        "echfs: Sectors per block: %u\n", drive->echfs_id_table->sectors_per_block;

        drive->echfs_id_table->dir_entries_per_block =
            drive->echfs_id_table->sectors_per_block * 2;

        drive->echfs_id_table->dev_cache_state = ECHFS_BLKDEV_CACHE_NOTREADY;
        drive->echfs_id_table->dev_cache       = MAlloc(ECHFS_BLKDEV_BLOCK_SIZE);

		drive->fs_type   = FSt_ECHFS;
        drive->root_clus = -1;

		if (unlock) {
			DriveUnlock(drive);
		}
	} catch {
		if (unlock) {
			DriveUnlock(drive);
		}
	}
}

U8 *EchFSFileWrite(CDrive *drive, U8 *cur_dir, U8 *filename,
				  U8 *buf, I64 size, CDate cdt, I64 attr) {
	"echfs write: %s %s\n", cur_dir, filename;
	//while (1) {}
}

U8 *EchFSFileRead(CDrive *drive, U8 *cur_dir, U8 *filename,
				  I64 *size, I64 *attr) {
	U8 *full_path, *buf, *p;
    CEchFSPathResult pres;
    U64 cur_block, fat_addr;
    CEchFSIdTable *id_tab = drive->echfs_id_table;

    full_path = MAlloc(StrLen(cur_dir) + 1 + StrLen(filename) + 1);
    StrCopy(full_path, cur_dir);
    if (cur_dir[StrLen(cur_dir)-1] != '/')
        StrCopy(full_path + StrLen(full_path), "/");
    StrCopy(full_path + StrLen(full_path), filename);

    if (!EchFSResolvePath(drive, &pres, full_path, ECHFS_TYPE_FILE, ROOT_DIR_ID)) {
        Free(full_path);
        return NULL;
    }

    Free(full_path);

    if (pres.not_found)
        return NULL;

    buf = MAlloc(CeilI64(pres.target.size, id_tab->bytes_per_block));
    p = buf;

    fat_addr = id_tab->fat_start * id_tab->bytes_per_block;

    for (cur_block = pres.target.payload; cur_block != END_OF_CHAIN;
         EchFSBlkDevRead(drive, &cur_block, fat_addr + cur_block * sizeof(U64),
                         sizeof(U64))) {
        EchFSBlkDevRead(drive, buf, cur_block * id_tab->bytes_per_block,
                        id_tab->bytes_per_block);
        buf += id_tab->bytes_per_block;
    }

    return p;
}

CDirEntry *EchFSFilesFind(U8 *files_find_mask, I64 fuf_flags, CDirEntry *parent=NULL) {
    "echfs files find\n";
    //while (1) {}
}

Bool EchFSFileFind(CDrive *drive,I64 cur_dir_clus,U8 *name,
			       CDirEntry *_res,I64 fuf_flags=0) {
    CEchFSPathResult pres;
    U64 type;

    type = ECHFS_TYPE_ANY;

    if (fuf_flags & FUF_JUST_DIRS && !(fuf_flags & FUF_JUST_FILES))
        type = ECHFS_TYPE_DIR;
    else if (fuf_flags & FUF_JUST_FILES && !(fuf_flags & FUF_JUST_DIRS))
        type = ECHFS_TYPE_FILE;

    if (!EchFSResolvePath(drive, &pres, name, type, cur_dir_clus(U64)))
        return FALSE;

    MemSet(_res, 0, sizeof(CDirEntry));

    MemCopy(_res->name, pres.target.name, CDIR_FILENAME_LEN - 1);
    _res->name[CDIR_FILENAME_LEN-1] = 0;
    _res->clus = pres.target.payload;
    _res->size = pres.target.size;

    return TRUE;
}