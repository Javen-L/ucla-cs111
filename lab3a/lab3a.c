#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ext2_fs.h"

typedef unsigned int uint;
typedef __u8 byte;
typedef __u8 bit;

#define SB_OFFSET           1024
#define DIR_NAME_LEN        6
#define DIR_MIN_LEN         8

int fs;
uint num_blocks;
uint num_inodes;
uint block_size;
uint inode_size;
uint inodes_per_group;
uint block_bitmap;
uint inode_bitmap;
uint inode_table;

int read_superblock() {
    struct ext2_super_block super_block;

    // Read superblock
    int retval = pread(fs, &super_block, sizeof(super_block), SB_OFFSET);
    if (retval < 0){
        fprintf(stderr, "error: cannot read at offset %u: %s\n", SB_OFFSET, strerror(errno));
        return 2;
    }

    // Check if it really is a superblock
    if (super_block.s_magic != EXT2_SUPER_MAGIC){
        fprintf(stderr, "error: bad ext2 filesystem\n");
        return 2;
    }

    num_blocks = super_block.s_blocks_count;
    num_inodes = super_block.s_inodes_count;
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    inode_size = super_block.s_inode_size;
    inodes_per_group = super_block.s_inodes_per_group;
    
    // Print superblock summary
    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
        num_blocks,
        num_inodes,
        block_size,
        inode_size,
        super_block.s_blocks_per_group,
        inodes_per_group,
        super_block.s_first_ino);

    return 0;
}

int read_group_descriptor(){
    struct ext2_group_desc group_desc;
    uint offset = SB_OFFSET + block_size;

    // Read group descriptor
    int retval = pread(fs, &group_desc, sizeof(group_desc), offset);
    if (retval < 0){
        fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
        return 2;
    }

    block_bitmap = group_desc.bg_block_bitmap;
    inode_bitmap = group_desc.bg_inode_bitmap;
    inode_table = group_desc.bg_inode_table;
        
    // Print group summary
    printf("GROUP,0,%u,%u,%u,%u,%u,%u,%u\n",
        num_blocks,
        num_inodes,
        group_desc.bg_free_blocks_count,
        group_desc.bg_free_inodes_count,
        block_bitmap,
        inode_bitmap,
        inode_table);

    return 0;
}

int free_entries(char tag){
    byte bitmap[block_size];
    uint iter = tag=='B'? num_blocks : num_inodes;
    uint bitmap_pos = tag=='B'? block_bitmap : inode_bitmap;
    uint offset = SB_OFFSET + (bitmap_pos - 1) * block_size;

    // Read bitmap
    int retval = pread(fs, bitmap, block_size, offset);
    if (retval < 0){
        fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
        return 2;
    }

    uint i,j;
    for (i=0; i<iter; i++){
        byte temp = bitmap[i];
        for (j=0; j<8; j++){
            bit temp_bit = temp & 1;
            if (temp_bit == 0)
                printf("%cFREE,%u\n", tag, i*8+j+1);
            temp = temp >> 1;
        }
    }
    return 0;
}

int read_directory_block(uint level, uint block_id, uint p_inode){
    int retval;
    uint offset = SB_OFFSET + (block_id - 1) * block_size;
    // Read directory entry
    if (level == 0){

        // Read entire block
        __u8 block[block_size];
        retval = pread(fs, &block, block_size, offset);
        if (retval < 0){
            fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
            return 2;
        }

        __u16 log_offset = 0;
        __u16 entry_size = 0;
        struct ext2_dir_entry dir_entry;
        while (log_offset < block_size){

            // Read directory entry
            entry_size = DIR_MIN_LEN + block[log_offset+DIR_NAME_LEN];
            memcpy(&dir_entry, block + log_offset, entry_size);

            // Invalid dir entry
            if (dir_entry.inode == 0)
                break;


            // Print direct dir entry
            printf("DIRENT,%u,%u,%u,%u,%u,'%.*s'\n",
                p_inode,
                log_offset,
                dir_entry.inode,
                dir_entry.rec_len,
                dir_entry.name_len,
                dir_entry.name_len,
                dir_entry.name);
            
            log_offset += dir_entry.rec_len;
            memset(&dir_entry, 0, entry_size);
        }
        return 0;
    }

    // Read the entire block
    __u32 block[block_size];
    retval = pread(fs, &block, block_size, offset);
    if (retval < 0){
        fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
        return 2;
    }

    uint i;
    for (i=0; i<block_size/4; i++){
        if (block[i] == 0)
            return 0;
        return read_directory_block(level-1, block[i], p_inode);
    }
    return 2;
}

int read_directory(const uint *inode_block, uint p_inode){
    int retval;
    int i;
    for (i=0; i<EXT2_N_BLOCKS; i++){
        if (inode_block[i] == 0)
            continue;

        /* 
         * level=0 -> direct            (i=0,1,...,11)
         * level=1 -> indirect single   (i=12)
         * level=2 -> indirect double   (i=13)
         * level=3 -> indirect triple   (i=14)
        */
        int level = i<12 ? 0 : i-11;

        retval = read_directory_block(level, inode_block[i], p_inode);
        if (retval > 0)
            return retval;
    }
    return 0;
}

int read_block(uint level, uint block_id, uint p_inode, uint log_offset){
    // In this project, we only need to print indirect blocks
    if (level == 0)
        return 0;

    __u32 block[block_size];
    uint offset = SB_OFFSET + (block_id - 1) * block_size;

    // Read the entire block
    int retval = pread(fs, &block, block_size, offset);
    if (retval < 0){
        fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
        return 2;
    }

    uint i;
    for (i=0; i<block_size/4; i++){
        if (block[i] == 0)
            continue;

        // Print indirect summary
        printf("INDIRECT,%u,%u,%u,%u,%u\n",
            p_inode,
            level,
            log_offset+i,
            block_id,
            block[i]);
            read_block(level-1, block[i], p_inode, log_offset);
    }
    return 0;
}

int read_indirect_data(const uint *inode_block, uint p_inode){
    int retval;
    uint used_block = 0;
    int i;
    for (i=EXT2_IND_BLOCK; i<EXT2_N_BLOCKS; i++){
        if (inode_block[i] == 0)
            continue;

        /* 
         * level=0 -> direct            (i=0,1,...,11)
         * level=1 -> indirect single   (i=12)
         * level=2 -> indirect double   (i=13)
         * level=3 -> indirect triple   (i=14)
        */
        uint level =  i<12 ? 0 : i-11;//EXT2_NDIR_BLOCKS

        uint log_offset = 11;
        uint j;
        for (j=1; j<=level; j++)
            log_offset += pow(block_size/4, j-1);

        retval = read_block(level, inode_block[i], p_inode, log_offset);
        if (retval > 0)
            return retval;
        used_block++;
    }
    return 0;
}

int read_inode_table(){
    int retval;
    uint offset = SB_OFFSET + (inode_table - 1) * block_size;

    uint inode_id;
    for (inode_id=1; inode_id<=num_inodes; inode_id++){
        struct ext2_inode inode;

        // Read inode
        retval = pread(fs, &inode, sizeof(inode), offset);
        if (retval < 0){
            fprintf(stderr, "error: cannot read at offset %u: %s\n", offset, strerror(errno));
            return 2;
        }

        if (inode.i_mode != 0 && inode.i_links_count != 0){
            
            // Get file type tag
            char file_type;
            if (S_ISREG(inode.i_mode)){
                file_type = 'f';
            }
            else if (S_ISDIR(inode.i_mode)){
                file_type = 'd';
            }
            else if (S_ISLNK(inode.i_mode)){
                file_type = 's';
            }
            else
                file_type = '?';

            // Create time tag
            time_t time_src[] = {inode.i_ctime, inode.i_mtime, inode.i_atime};
            char time_tag[3][18];
            int i;
            for (i=0; i<3; i++){
                struct tm *p_tm = gmtime(&time_src[i]);
                if(p_tm == NULL){
                    fprintf(stderr, "error: call to 'gmtime' failed: %s\n", strerror(errno));
                    return 2;
                }
                retval = strftime(time_tag[i], 18, "%m/%d/%y %H:%M:%S", p_tm);
                if(retval != 17){
                    fprintf(stderr, "error: 'strftime' returned a wrong time_tag format\n");
                    return 2;
                }
            }

            // Print inode summary
            printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",
                inode_id,
                file_type,
                inode.i_mode & 0xFFF,
                inode.i_uid,
                inode.i_gid,
                inode.i_links_count,
                time_tag[0],
                time_tag[1],
                time_tag[2],
                inode.i_size,
                inode.i_blocks);
   
            // Print block addresses
            if (file_type == 's' && inode.i_size < 60){
                printf("\n");
            }
            else if (file_type != '?'){
                for(i=0; i<EXT2_N_BLOCKS-1; i++){
                    printf(",%u", inode.i_block[i]);
                }
                printf(",%u\n", inode.i_block[EXT2_N_BLOCKS-1]);
            }
            else {
                printf("\n");
            }

            // Print directory entries
            if (file_type == 'd'){
                retval = read_directory(inode.i_block, inode_id);
                if(retval < 0)
                    return retval;
            }

            // Print indirect entries
            if (file_type == 'd' || file_type == 'f'){
                retval = read_indirect_data(inode.i_block, inode_id);
                if(retval < 0)
                    return retval;
            }
        }
        offset += inode_size;
    }
    return 0;
}

int main(int argc, char **argv){
    int retval;

    if (argc != 2){
        fprintf(stderr, "usage: %s filesystem_image\n", argv[0]);
        exit(1);
    }
    
    // Open filesystem
    fs = open(argv[1], O_RDONLY);
    if (fs < 0){
        fprintf(stderr, "error: cannot open '%s': %s\n", argv[1], strerror(errno));
        exit(1);
    }

    // SUPPERBLOCK SUMMARY
    retval = read_superblock();
    if (retval > 0)
        exit(retval);

    // GROUP SUMMARY
    retval = read_group_descriptor();
    if (retval > 0)
        exit(retval);

    // FREE BLOCK ENTRIES
    retval = free_entries('B');
    if (retval > 0)
        exit(retval);

    // FREE INODE ENTRIES
    retval = free_entries('I');
    if (retval > 0)
        exit(retval);

    // INODE SUMMARY & DIRECTORY ENTRIES & INDIRECT BLOCK REFERENCE
    retval = read_inode_table();
    if (retval > 0)
        exit(retval);

    close(fs);
    exit(0);
}



