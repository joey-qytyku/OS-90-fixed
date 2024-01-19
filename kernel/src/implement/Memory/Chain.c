///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <Memory/MemDefs.h>
#include <Memory/Chain.h>

#include <Misc/log2.h>
#include <Misc/Linker.h>

#include <Scheduler/V86M.h>

static U32    num_total_blocks;
static P_MB   block_list;

// Size of PBT is dynamic and determined by checking size of physical RAM.

// Is this a good idea?
U32 Round_Bytes_To_Blocks(U32 bytes)
{
    return bytes >> BITS_NEEDED(MEM_BLOCK_SIZE) + (bytes & MEM_BLOCK_SIZE - 1);
}

// BRIEF:
//      Allocate a chain of blocks.
//
// EXPLAINATION:
//
// INPUTS:
//      num_pages:  Automatically converted to blocks. The program does not
//                  need to know how large a block is.
// FLAGS:
//
//  The following flags are provided for this function that decide attributes
//  about the block.
//
//  CH_USER             When we map this memory, remember to make user pages.
//                      Do not permit userspace to modify if CH_USER not passed.
//
// The privilege of the chain matters because a userspace program cannot
// manipulate chains that belong to the kernel.
//
// RETURN:
//      Allocated chain or INVALID_CHAIN
//
kernel CHID Chain_Alloc(
    U32 bytes,
    PID owner_pid
){
    // This is a value that we write to when allocating the first block.
    // A pointer is used to access the front link of the previous block,
    // but there is no special case for the first one except for the
    // first flag, so we need a place to put the irrelevant loop iterator
    // value.

    MB   nonsense_mb = { 0 };
    P_MB prev_blk = &nonsense_mb;

    U32 num_blocks_to_alloc = RoundBytesToBlocks(bytes);
    U32 base; // Index of first block


    // Find the first free block. It will be allocated by
    // the next loop.
    for (U32 i = 0; i < num_total_blocks; i++)
    {
        if (block_list[i].f_free)
            base = i;
    }

    U32 prev_index = 0;

    // Allocate every block. Iterator starts at base
    for (U32 i = 0; i < num_total_blocks; i++)
    {
        const U32 abs_index = base + i;
        const P_MB  curr_blk  = &block_list[abs_index];

        // The last in the list will have zero as the default
        // next link. This will not be changed for last entry
        // and remains zero, which is not valid for the next
        // entry.
        curr_blk->next = 0;

        curr_blk->f_free      = 0;
        curr_blk->owner_pid   = owner_pid;
        curr_blk->rel_index   = i;

        // Set the front link of the previous MB to point to
        // the current one
        prev_blk->next = base + i;
        // Set the back link of the current one to reference the previous
        // block
        curr_blk->prev = prev_index;

        // Update previous index for next iteration
        prev_index = i;
    }
    // THIS WILL FALL THROUGH!
    // oom is not used.
oom:
    // Add OOPM handler?
    // No. We will demand page with separate code that will check INVALID_CHAIN
    // and act appropriately.
    return INVALID_CHAIN;
good:
    return base;
}

BOOL Chain_Is_Valid(CHID chain)
{
    if (block_list[chain].f_free == 1 && block_list[chain].rel_index == 0)
        return 1;
    return 0;
}

// BRIEF:
//      Return size of chain in bytes.
//
// RETURN:
//      A chain cannot be zero bytes long, so a return value of zero is invalid.
//      Valid return value will *always* be block granular.
//
U32 Chain_Size(CHID chain)
{
    U32 curr_inx   = chain;
    U32 byte_count = 0;

    if (!ChainIsValid(chain))
        goto End;

    // do while?
    do {
        byte_count += MEM_BLOCK_SIZE;
        curr_inx = block_list[curr_inx].next;
    }
    while (block_list[curr_inx].next > 0);
End:
    return byte_count;
}

// BRIEF:
//      Allocate a contiguous range of blocks. This is done toward the end of
//      extended memory for simplicity.
//
CHID Chain_Alloc_Physical_Contig()
{}

typedef BOOL (*ITERATE_CHAIN_FUNC)(P_MB,U32);

// BRIEF:
//      Run this function for each entry in this chain.
//
// RETURN:
//      A pointer to the last block entry checked.
//
static P_MB For_Each_Block_In_Chain(
    ITERATE_CHAIN_FUNC _do,
    CHID               id,
    U32                extra
){
    P_MB block;
    // Pass it the index. If it returns 1, break.
    for (block = &block_list[id]; ;)
    {
        if (_do(block, extra))
            break;
        block = &block_list[block->next];
    }
    return block;
}

static BOOL Slant_Indices_By_Addend(P_MB block, U32 addend)
{
    block->rel_index += (U16)addend;
}

static BOOL Check_Last(P_MB block, U32 dummy)
{
    UNUSED_PARM(dummy);
    return (BOOL)block->next;
}

// Way to speed up this calculation?
static U32 Get_Index_Of_Last_Entry(CHID id)
{
    const U32 last_block_2int = (U32)For_Each_Block_In_Chain(Check_Last, id, 0);
    const U32 block_list_2int = (U32)block_list;

    return (last_block_2int - block_list_2int) / sizeof(MB);
}

//
// Scale the chain by the provided value. This operation is relative only.
//
// Uncommitted memory is partially kept track of in the block table.
// curr_block.index - prev_block.index - 1 = Uncommitted blocks between
//
// The only issue is that committed blocks must surround it or there will be no
// way to detect the presence of uncommitted blocks.
//
// That is why the function takes the uncommitted and committed size.
// If committed is 0, then we have to commit blocks anyway.
//

STATUS kernel Chain_Extend(
    CHID    id,
    U32     bytes_uncommit,
    U32     bytes_commit
){
    U32 blocks_commit   = RoundBytesToBlocks(bytes_commit);
    U32 blocks_uncommit = RoundBytesToBlocks(bytes_uncommit);

    // If the chain is invalid or no memory is being allocated
    // return error.

    if (!ChainIsValid(id) || (bytes_commit + bytes_uncommit) == 0)
        return OS_ERROR_GENERIC;

    if (blocks_commit == 0)
    {
        // The number of uncommitted blocks minus one will be generated
        blocks_uncommit--;
        // We will commit at least one block at the end.
        blocks_commit = 1; //++?
    }

    const U32 blocks_in_chain = ChainSize(id) / MEM_BLOCK_SIZE;

    // The last block allocated before extention. To find it, we have to
    // iterate blocks_in_chain times
    const U32 final_block_before_ext = GetIndexOfLastEntry(id);

    // BOOL is_user = (block_list[id].owner_pid != 0);

    // We will use the chain alloc function to get a new chain. This will
    // be linked to the previous one later.
    const CHID ext_chain = ChainAlloc(blocks_commit * MEM_BLOCK_SIZE, 0);

    if (ext_chain == INVALID_CHAIN)
        return OS_ERROR_GENERIC;

    // Slant the indices forward by the number of uncommitted

    For_Each_Block_In_Chain(
        Slant_Indices_By_Addend,
        ext_chain,
        blocks_uncommit
    );

    // Link the provided chain and the previous one. This will merge the two
    block_list[final_block_before_ext].next = ext_chain;
    block_list[ext_chain             ].prev = final_block_before_ext;

    return OS_OK;
}

// BRIEF:
//      Returns the physical address of a block in a chain.
//      Works by following the linked list.
//
// RETURN:
//      Address of the block in the chain
//      NULL if does not exist.
//
// Best if I unroll this.
//
PVOID Chain_Walk(
    CHID    id,
    U32     req_index
){
    P_MB block = &block_list[id];
    U32 i = 0;

    for (; i < req_index + 1 ;i++)
    {
        // Return the address if we found the entry.

        if (block->rel_index == req_index)
            return block;

        // If there is no next block, do not continue.

        if (block_list->next == 0)
            break;

        // Go to next block
        block = &block_list[block->next];
    }
    // If the loop finished, it means that all blocks in this chain
    // were exhausted and this entry does not exist.
    return NULL;
}

// Stack trace info can be variable size and is independent of the linker?
VOID Chain_Init(VOID)
{
    // const U32 lkr_end_int = (U32)&; // minus something?
    // const U32 mbsize = MEM_BLOCK_SIZE;
    // block_list = (lkr_end_int + mbsize-1) & (~(mbsize-1));

    // We must determine how many free blocks there are. This requires V86.

}
