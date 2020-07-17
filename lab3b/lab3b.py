import sys
import csv
import os
import errno
import math
import pandas as pd

SUPERBLOCK_OFFSET = 1024
TABLE_DESC_SIZE = 32
ROOT_INODE = 2

def block_consistency_audit():
    retval = 0
    inode_table_size = math.ceil(sb.i_count * sb.i_size / sb.b_size)
    reserved_size = 4 + inode_table_size
    data_blocks = range(reserved_size+1, sb.b_count)
    i_block = inodes[inodes.columns[-15:]]
    block_type = dict.fromkeys(range(12), "BLOCK")
    block_type[12] = "INDIRECT BLOCK"
    block_type[13] = "DOUBLE INDIRECT BLOCK"
    block_type[14] = "TRIPLE INDIRECT BLOCK"
    referenced = []
    inode_id = []
    offsets = []
    indices = []

    ## Checck invalid/reserved blocks
    for index, inode in inodes.iterrows():
        block_id = inode[-15:]
        offset = -1

        for idx in range(15):
            # Compute offset
            if idx < 12:
                offset += 1
            else:
                offset = 11
                for i in range(1, idx-10):
                    offset += int(pow(sb.b_size/4, i-1))

            if block_id[idx] == 0:
                continue
            else:
                referenced.append(block_id[idx])
                inode_id.append(inode.id)
                offsets.append(offset)
                indices.append(idx)

            # Invalid
            if block_id[idx] < 0 or block_id[idx] > sb.b_count:
                print("INVALID {} {} IN INODE {} AT OFFSET {}"
                        .format(block_type[idx], block_id[idx], inode.id, offset))
                retval = 2
            # Reserved
            if block_id[idx] in range(1,reserved_size):
                print("RESERVED {} {} IN INODE {} AT OFFSET {}"
                        .format(block_type[idx], block_id[idx], inode.id, offset))
                retval = 2

    ## Check unreferenced data block
    for block_id in data_blocks:
        if block_id not in referenced\
            and block_id not in free_blocks\
            and block_id not in indirect.ref_block.tolist():
            print("UNREFERENCED BLOCK {}".format(block_id))
            retval = 2
    
    ## Check allocated block in the free block list
    for block_id in list(set(free_blocks) & set(referenced + indirect.ref_block.tolist())):
        print("ALLOCATED BLOCK {} ON FREELIST".format(block_id))
        retval = 2

    ## Check multiply referenced blocks
    for idx in range(len(referenced)):
        temp = referenced.copy()
        block_id = temp.pop(idx)
        if block_id in indirect.ref_block.tolist()\
            or block_id in temp:
            print("DUPLICATE {} {} IN INODE {} AT OFFSET {}"
                    .format(block_type[indices[idx]], block_id, inode_id[idx], offsets[idx]))
            retval = 2
    return retval

def inode_allocation_audit():
    retval = 0
    usable_inode = range(sb.first_avail, sb.i_count)
    allocated = inodes.id.tolist()
    unallocated = [i for i in usable_inode if i not in allocated]

    for inode in allocated:
        if inode in free_inodes:
            print("ALLOCATED INODE {} ON FREELIST".format(inode))
            retval = 2

    for inode in unallocated:
        if inode not in free_inodes:
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(inode))
            retval = 2

    return retval

def directory_consistency_audit():
    retval = 0

    ## Check unmatched link count
    links = dir_entries.ref_inode.value_counts()
    for inode in inodes.id:
        linkcount = inodes.loc[inodes.id==inode].iloc[0].link_count
        if inode not in links.index:
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}"
                    .format(inode, 0, linkcount))
            retval = 2
        elif links[inode] != linkcount:
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}"
                    .format(inode, links[inode], linkcount))
            retval = 2      

    ## Check dir inode validity
    for index, entry in dir_entries.iterrows():
        if entry.ref_inode < 1 or entry.ref_inode > sb.i_count:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}"
                    .format(entry.dir_inode,entry['name'],entry.ref_inode))
            retval = 2

        elif entry.ref_inode not in inodes.id.tolist():
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}"
                    .format(entry.dir_inode,entry['name'],entry.ref_inode))
            retval = 2

        ## Check if '.' is correct
        if entry['name'] == "'.'":
            if entry.dir_inode != entry.ref_inode:
                print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}"
                    .format(entry.dir_inode,entry.ref_inode,entry.dir_inode))
                retval = 2

        ## Check if '..' is correct
        if entry['name'] == "'..'":
            if entry.dir_inode == ROOT_INODE:
                if entry.ref_inode != ROOT_INODE:
                    print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}"
                        .format(entry.dir_inode,entry.ref_inode,ROOT_INODE))
                    retval = 2
            else:
                # Determine the correct parent directory
                potential_parent = dir_entries.loc[(dir_entries.ref_inode==entry.dir_inode) 
                        & (dir_entries.dir_inode!=entry.dir_inode)].dir_inode.tolist()
                parent_dir = list(set(potential_parent))[0]

                if entry.ref_inode != parent_dir:
                    print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}"
                        .format(entry.dir_inode,entry.ref_inode,parent_dir))
                    retval = 2
    return retval


### READ INPUT FILE ###
if not len(sys.argv) == 2:
    print("usage: {} /path/to/filesystem_summary.csv".format(sys.argv[0]), file=sys.stderr)
    exit(1)
try:
    fs = open(sys.argv[1])
    df = pd.DataFrame(csv.reader(fs))
    fs.close()
except OSError as err:
    print("error: 'open' failed: {}".format(err.strerror), file=sys.stderr)
    exit(1)

### PROCESS DATA ###
sb = df.loc[df[0]=="SUPERBLOCK"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric).iloc[0]
sb.index=["b_count","i_count","b_size","i_size","b_per_g","i_per_g","first_avail"]

group = df.loc[df[0]=="GROUP"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric).iloc[0]
group.index=["id","b_count","i_count","free_b_count","free_i_count","addr_b_bitmap","addr_i_bitmap","i_table"]

inodes = df.loc[df[0]=="INODE"].fillna(0).drop(0,axis=1).apply(pd.to_numeric, errors="ignore")
inodes.columns=["id","type","mode","owner","group","link_count","ctime","mtime","atime","file_size","block_count",
                "b1","b2","b3","b4","b5","b6","b7","b8","b9","b10","b11","b12","b13","b14","b15"]
directories = inodes.loc[inodes.type=="d"].id.tolist()

indirect = df.loc[df[0]=="INDIRECT"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric)
indirect.columns=["p_inode","level","offset","block_id","ref_block"]

dir_entries = df.loc[df[0]=="DIRENT"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric, errors="ignore")
dir_entries.columns=["dir_inode","offset","ref_inode","entry_len","name_len","name"]

free_blocks = df.loc[df[0]=="BFREE"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric).squeeze().tolist()
free_inodes = df.loc[df[0]=="IFREE"].dropna(axis=1).drop(0,axis=1).apply(pd.to_numeric).squeeze().tolist()

### RUN AUDIT ###
retval = block_consistency_audit()
retval = inode_allocation_audit()
retval = directory_consistency_audit()

exit(retval)
