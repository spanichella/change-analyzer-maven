/* Copyright       : OHB System AG 
 * Project         : SCSW 
 * File            : MemPool.c 
 * Model Component : MemPool 
 * Description     : 
 * This class provides the interface to a MemoryPool.
                                                                                                                      */

#include "MemPool.h"
#include <stddef.h>
/*## package BasicServicesPkg::MemPoolPkg */

/*## class MemPool */
/* Name        : getMemBlock
 * Description :
 * This method returns the index of a physical memory block of the requested size.                                    */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_UInt16_t. */
/*     - Index of the memory block of the requested size. */
/*     - NULL_BLOCK: if no block of the requested size is available. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - Search the first FreeList containing a free MemBlock from the "itsFreeList" array (attribute nNbElemInList of    */
/* the FreeList different than 0) . It starts searching in the "n8InSize" list continuing with bigger size lists if   */
/* it does not find free MemBlocks.                                                                                   */
/* - If a FreeList is found: */
/*     - It gets the Index of the last MemBlock in the list from the FreeList.nLast attribute.  */
/*     - It is removed from the FreeList with the FreeList_remove method. */
/*     - The sHeader.n8Status of the MemBlock in the Index position in the itsMemBlock array is set to                */
/*     BLOCK_STS_RESERVED.                                                                                            */
/*     - If the block is obtained from a list of blocks of bigger size than requested in n8InSize, it is divided      */
/*     using the method divideBlock, and the obtained buddy added to the corresponding itsFreeList with the method    */
/*     FreeList_add.                                                                                                  */
/*     - This division is repeated as many times as necessary to obtain a block of the requested size. */
/*         - If a MemBlock of n8Size=0 shall be added to the FreeList its status is set to BLOCK_STS_BUDDY1_FREE. */
/*     - Compute the physical block that will be used by multiplying by 2 the index of the MemBlock. */
/*     - If n8Size=0, and the MemBlock got from the freeList had the status BLOCK_STS_BUDDY1_FREE, the returned block */
/*     is increased by one.                                                                                           */
/*     - The computed physical block is returned.     */
/* - Else return NULL_BLOCK */
/*     */
/*  
 * Arguments   :
 *     n8InSize - In - MemBlock_Size_t - Size of the requested block (value between 0  and n8K). The value is not 
checked. */
/* 
 */
/*## operation getMemBlock(Size_t) const */
static Ohb_UInt16_t getMemBlock(const MemPool* const me, MemBlock_Size_t n8InSize);

/* Name        : freeMemBlock
 * Description :
 * This method frees the block passed as input argument, and combines it with its buddies.                            */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_Error_t. */
/*     - Error_None: No error */
/*     - Error_Argument: The block passed as argument is not being used. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - If the status of the block passed as argument is not BLOCK_STS_FREE. */
/*      */
/*     - Gets the index of its buddy with the method getBuddy */
/*  */
/*     - Starts the process of recombination of blocks, consisting on: */
/*         - If size of the block is the same as the one of its buddy, and the buddy status is not                    */
/*         BLOCK_STS_RESERVED:                                                                                        */
/*             - Removes the buddy from the FreeList with the method FreeList_remove. */
/*             - Combines the two blocks with the method combineBlocks. */
/*             - Gets the buddy of the combined block and starts the process again. */
/*         - If the sizes are different or the status of the buddy is not free: */
/*             - The recombination process is terminated */
/*     - Updates the status of the the last MemBlock obtained on the recombination process. */
/*         - If the returned MemBlock is of size 0  */
/*             - If the physical memory block was an odd block, the status is set to BLOCK_STS_BUDDY1_FREE */
/*             - If the physical memory block was an even block, the status is set to BLOCK_STS_BUDDY0_FREE */
/*         - For any other size the status is set to BLOCK_STS_FREE */
/*     - The MemBlock is added to the corresponding free list with the method FreeList_add.     */
/*     - Returns with Error_None. */
/*  */
/* - Else return Error_Argument  */
/*  */
/*  */
/*  */
/*  
 * Arguments   :
 *     n16MemBlock - In - Ohb_UInt16_t - Index of the physical memory block to be frred from the MemPool. The 
function assumes that the index is correct. */
/* 
 */
/*## operation freeMemBlock(Ohb_UInt16_t) const */
static Ohb_Error_t freeMemBlock(const MemPool* const me, Ohb_UInt16_t n16MemBlock);

/* Name        : combineBlocks
 * Description :
 * This method combines the two blocks passed as argument. */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_UInt16_t. */
/*     - Index of the combined block. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - Increase the size attribute of both blocks by one. (attribute sHeader.n8Size) */
/* - Sets the status of both blocks to BLOCK_STS_FREE (attribute sHeader.n8Status) */
/* - The lowest value of the 2 input arguments is returned as index to the combined block. 
 * Arguments   :
 *     n16Block1 - In - Ohb_UInt16_t - Index of the first block to combine.                                           */
/* The validity is not check since the method is called internally and checks are done before. */
/*  */
/*     n16Block2 - In - Ohb_UInt16_t - Index of the second block to combine. */
/* The validity is not check since the method is called internally and checks are done before. */
/* 
 */
/*## operation combineBlocks(Ohb_UInt16_t,Ohb_UInt16_t) const */
static Ohb_UInt16_t combineBlocks(const MemPool* const me, Ohb_UInt16_t n16Block1, Ohb_UInt16_t n16Block2);

/* Name        : divideBlock
 * Description :
 * This method divides the block passed as argument. */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_UInt16_t. */
/*     - Index of the buddy obtained after division. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - Decrease the size attribute of the input block by one. (attribute sHeader.n8Size) */
/* - Obtains the index of its buddy with the method getBuddy */
/* - Sets the size of the buddy to the same as the input block. */
/* - Sets the status of the buddy to BLOCK_STS_FREE (attribute sHeader.n8Status) */
/*  */
/*  
 * Arguments   :
 *     n16Block - In - Ohb_UInt16_t - Index of the block to divide. No checks are done in this argument since it is a 
private function. */
/* 
 */
/*## operation divideBlock(Ohb_UInt16_t) const */
static Ohb_UInt16_t divideBlock(const MemPool* const me, Ohb_UInt16_t n16Block);

/* Name        : getBuddy
 * Description :
 * This method returns the buddy of the block passed as argument. */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_UInt16_t. */
/*     - Index of the buddy. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - If the sStatus.n8size of the MemBlock is 0 the index of the buddy MemBlock is the same as the input. */
/*  */
/* - Else, computes the index of the buddy by swapping the bit in the "sStatus.n8size - 1" position of the block      */
/* index.                                                                                                             */
/*      - ex. If block Index = 0 and Size 2  -> buddy = 2 
 * Arguments   :
 *     n16Block - In - Ohb_UInt16_t - Index of the block which its buddy is requested. No checks are done in this     */
/*      argument since it is a private function.                                                                      */
/* 
 */
/*## operation getBuddy(Ohb_UInt16_t) const */
static Ohb_UInt16_t getBuddy(const MemPool* const me, Ohb_UInt16_t n16Block);

/* Name        : getAddress
 * Description :
 * This method returns the memory address corresponding to the block passed as argument..                             */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_UInt8_t *. */
/*     - Pointer to the memory address corresponding to the block passed as argument. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PROCESSING# */
/* - The method computes the memory address inside the pData byte array corresponding to the block passed as          */
/* argument.                                                                                                          */
/* - This computation is done using the block index passed as argument, the nMinBlockSize parameter and the pData     */
/* start position.                                                                                                    */
/* - Returns : pData + (n16Block *  nMinBlockSize).  
 * Arguments   :
 *     n16Block - In - Ohb_UInt16_t - Index of the memory block which its address is requested. The validity of the   */
/* argument is not checked.                                                                                           */
/* 
 */
/*## operation getAddress(Ohb_UInt16_t) const */
static Ohb_UInt8_t * getAddress(const MemPool* const me, Ohb_UInt16_t n16Block);

/* Name        : initialise
 * Description :
 * This method initialises the class attributes. */
/*  */
/* #RETURN_VALUE_DESCRIPTION# */
/* Returns an Ohb_Error_t. */
/*     - Error_None: operation success. */
/*     - Error_Failed: if there are incoherent attributes in the class. */
/*  */
/* #THREAD_SAFETY# */
/* This operation is NOT thread-safe. */
/*  */
/* #PRECONDITIONS# */
/* The attributes pData, nNbBytes, n8KMax, n8KMin, n8K, n16NbBlocks, itsFreeList and itsMemBlock shall be initialised */
/* before calling this method.                                                                                        */
/*  */
/* #PROCESSING# */
/* - Initialises nMinBlockSize to 2^n8KMin. */
/* - Initialises all the n8K + 1 freeLists (attribute itsFreeList) calling the method FreeList_init for each list,    */
/* passing as argument the attribute itsMemBlock.                                                                     */
/* - If there is at least one block of maximum size (nNbBytes / 2^n8KMax >= 1) and the attribute n16NbBlocks is equal */
/* to ((me->nNbBytes / me->nMinBlockSize)/2).                                                                         */
/*     - Initialises the maximum size MemBlocks and adds them to the freeList[n8K] (the list containing the max size  */
/*     free blocks)                                                                                                   */
/*         - Sets the attribute sHeader.n8Size of the block to n8K.  */
/*         - Sets the attribute sHeader.eStatus of the block to BLOCK_STS_FREE. */
/*         - Adds the block to the free list with the method FreeList_add        */
/*     - Returns Error_None. */
/* - Else return Error_Failed. (This error is produced when the  nNbBytes or n16NbBlocks has not been properly        */
/* initialised).                                                                                                      */
/*  
 * Arguments   :
 * 
 */
/*## operation initialise() */
static Ohb_Error_t initialise(MemPool* const me);

/*## operation init() */
Ohb_Error_t MemPool_init(MemPool* const me) {
    /*#[ operation init() */
    Ohb_Error_t eReturnValue = Error_None;
    
    
    /* Check that the class attributes are properly initialised */
    
    /* Check that pointers are correct */
    /* polyspace<RTE: UNR : Not a defect : No action planned > defensive null pointer check, see CCM ID 6918 */
    if ((me->itsFreeList == NULL) || (me->itsMemBlock == NULL) || (me->pData == NULL) ){
    
    	eReturnValue = Error_Failed;
    }
    /* Check that KMAX and KMIN are inside ABSOLUTE bounds and consistent */
    else if ((me->n8KMin < K_MIN_ABSOLUTE) || (me->n8KMax < me->n8KMin) || (me->n8KMax > K_MAX_ABSOLUTE)) {
    
    	eReturnValue = Error_Failed;
    }
    /* Check that K is consistent with KMAX and KMIN */
    else if ((me->n8K != (me->n8KMax - me->n8KMin))){
    
    	eReturnValue = Error_Failed;
    }
    else {
    
    	/* If all the chacks are passed initialise the rest of MemPool attributes */
    	eReturnValue = initialise(me);
    }
    
    return eReturnValue;
    /*#]*/
}

/*## operation allocate(Ohb_VoidPtr_t,Ohb_UInt32_t) const */
Ohb_Error_t MemPool_allocate(const MemPool* const me, Ohb_VoidPtr_t * const pAllocatedMem, Ohb_UInt32_t nInSize) {
    /*#[ operation allocate(Ohb_VoidPtr_t,Ohb_UInt32_t) const */
    Ohb_Error_t eReturnValue = Error_Argument;
    Ohb_UInt16_t n16Block;
    Ohb_UInt32_t nBlockSizeInBytes = (Ohb_UInt32_t)1u << me->n8KMin; /* Size in bytes of the smallest block*/
    MemBlock_Size_t eBlockSize;
    
    /*Input params check*/
    if ((pAllocatedMem != NULL) && (nInSize > 0u)) {
        /* Selects the block size*/
        for (eBlockSize = 0u; eBlockSize <= me->n8K; eBlockSize++) {
            if (nInSize <= nBlockSizeInBytes)  {
                /* If the size is valid obtains the memory block*/
                n16Block = getMemBlock(me,eBlockSize);
                if (n16Block != NULL_BLOCK){
                    *pAllocatedMem = getAddress(me,n16Block);
                    eReturnValue = Error_None;
                }
                else {
                    eReturnValue = Error_NoResources;
                    *pAllocatedMem = NULL;
                }
                break;
            }
            nBlockSizeInBytes = nBlockSizeInBytes << 1u;
        }
        /* If the size is bigger than the biggest block size the Error_Argument is returned*/
    }
    else
    {
        /*If the input pointer is null or the size is 0*/
        eReturnValue = Error_Argument;
    }
    
    return eReturnValue;
    
    /*#]*/
}

/*## operation free(Ohb_VoidPtr_t const) const */
Ohb_Error_t MemPool_free(const MemPool* const me, Ohb_VoidPtr_t const pMemToFree) {
    /*#[ operation free(Ohb_VoidPtr_t const) const */
    Ohb_Error_t eReturnValue = Error_Argument;
    Ohb_UInt16_t n16BlockIndex;
    Ohb_Int32_t iOffset =( (Ohb_Int32_t)pMemToFree - (Ohb_Int32_t)me->pData );
    
    /* Checks that the pointer is inside the memPool */
    if ((iOffset >= 0) && (iOffset <= (Ohb_Int32_t)me->nNbBytes)) {
        /* Computes the block index shifting KMin positions, that is equivalent to dividing by min block size */
        n16BlockIndex = (Ohb_UInt16_t)((Ohb_UInt32_t)iOffset >> me->n8KMin);
        eReturnValue = freeMemBlock(me,n16BlockIndex);
    }
    
    return  eReturnValue;
    /*#]*/
}

/*## operation getFreeBytes() const */
Ohb_UInt32_t MemPool_getFreeBytes(const MemPool* const me) {
    /*#[ operation getFreeBytes() const */
    Ohb_UInt32_t nFreeBytes = 0U;
    Ohb_UInt32_t nBlockSize;
    Ohb_UInt32_t nIndex;
    
    // For each possible block size (K+1)
    for (nIndex = 0U; nIndex <= me->n8K; nIndex++) {
    
        // Compute the current block size, in bytes
        nBlockSize = (Ohb_UInt32_t) 1U << (me->n8KMin + nIndex);
    
        // Accumulate the free bytes for the current block size
        nFreeBytes += nBlockSize * me->itsFreeList[nIndex].nNbElemInList;
    
    }
    
    return nFreeBytes;
    /*#]*/
}

/*## operation getMemBlock(Size_t) const */
static Ohb_UInt16_t getMemBlock(const MemPool* const me, MemBlock_Size_t n8InSize) {
    /*#[ operation getMemBlock(Size_t) const */
    
    Ohb_UInt16_t n16OutBlock = (Ohb_UInt16_t)NULL_BLOCK;
    Ohb_Int8_t i8SearchSize = (Ohb_Int8_t)n8InSize;
    Ohb_Int8_t i8DivideSize;
    Ohb_UInt16_t n16Buddy;
    Ohb_Boolean_t bIsBuddy1;
    
    
    /*Check if free locations available*/
    for (i8SearchSize = (Ohb_Int8_t)n8InSize; i8SearchSize <= (Ohb_Int8_t)me->n8K; i8SearchSize++) {
        if (me->itsFreeList[i8SearchSize].nNbElemInList != 0u){
    
            /* If there is a free element on the list*/
    
            /* Gets the last block from the list */
            n16OutBlock = me->itsFreeList[i8SearchSize].n16Last;
            FreeList_remove(&me->itsFreeList[i8SearchSize], n16OutBlock);
            
            /* Each block represents a pair of buddies, checks which one is the free and reserves the complete block */
            bIsBuddy1 = (me->itsMemBlock[n16OutBlock].sHeader.n8Status == BLOCK_STS_BUDDY1_FREE);
            me->itsMemBlock[n16OutBlock].sHeader.n8Status = BLOCK_STS_RESERVED;
            
            /* Divides the block the necessary times to have a block of the required size,
             * the buddies are put in the corresponding lists of free blocks according the size.*/
    
            /* The nDivideSize is casted to signed to detect the >= condition in case nInSize is 0*/
            for (i8DivideSize = i8SearchSize-1; i8DivideSize >= (Ohb_Int8_t)n8InSize; i8DivideSize--){
                n16Buddy = divideBlock(me, n16OutBlock);
                if (i8DivideSize == 0) {
                    me->itsMemBlock[n16Buddy].sHeader.n8Status = BLOCK_STS_BUDDY1_FREE;
                }
                FreeList_add(&me->itsFreeList[i8DivideSize],n16Buddy);
            }
    
            /* Compute the physical block that will be used by multiplying by 2 the index of the MemBlock given */
    		n16OutBlock = (n16OutBlock << 1u);
    		
    		/* Adding 1 in case the used block corresponds to the buddy1(odd block) of the pair of buddys represented
         in the MemBlock */
    		if (bIsBuddy1){
    		    n16OutBlock++;
    		}
    
            break;
        }
    }
    
    
    
    
    return n16OutBlock;
    
    /*#]*/
}

/*## operation freeMemBlock(Ohb_UInt16_t) const */
static Ohb_Error_t freeMemBlock(const MemPool* const me, Ohb_UInt16_t n16MemBlock) {
    /*#[ operation freeMemBlock(Ohb_UInt16_t) const */
    
    Ohb_Error_t eReturnValue = Error_Argument;
    MemBlock_Size_t n8TmpSize;
    Ohb_UInt16_t n16Buddy;
    Ohb_UInt16_t n16BlockIndex;
    Ohb_UInt16_t n16TmpBlock;
    Ohb_UInt16_t bIsBuddy1;
    
    /* Masks the last bit to know if the returned block is odd*/
    bIsBuddy1 = n16MemBlock & 1u;
    /* Divides the block by two to obtain the actual MemBlock object index*/
    n16BlockIndex = n16MemBlock >> 1u;
    
    n16TmpBlock = n16BlockIndex;
    
    
    /* If the block is not reserved returns error*/
    if(me->itsMemBlock[n16BlockIndex].sHeader.n8Status != BLOCK_STS_FREE) {
    
    
        /* Checks if the returned block can be combined with its buddy recursively */
        n16Buddy=getBuddy(me,n16BlockIndex);
    
    
        for( n8TmpSize = me->itsMemBlock[n16BlockIndex].sHeader.n8Size; n8TmpSize < me->n8K ; n8TmpSize++) {
    
            /* If the buddy is not configured with the same size or it is not free
             * the recombination process is stopped. */
            if ( ( me->itsMemBlock[n16Buddy].sHeader.n8Size != n8TmpSize ) ||
                 ( me->itsMemBlock[n16Buddy].sHeader.n8Status == BLOCK_STS_RESERVED ) ){
                break;
            }
            else {
                FreeList_remove(&me->itsFreeList[n8TmpSize],n16Buddy);
                n16TmpBlock = combineBlocks(me,n16TmpBlock,n16Buddy);
                n16Buddy=getBuddy(me,n16TmpBlock);
            }
        }
    
        /* Updates the block status */
        if (n8TmpSize == 0u) {
            if (bIsBuddy1 != 0u) {
                me->itsMemBlock[n16TmpBlock].sHeader.n8Status = BLOCK_STS_BUDDY1_FREE;
            } else {
                me->itsMemBlock[n16TmpBlock].sHeader.n8Status = BLOCK_STS_BUDDY0_FREE;
            }
        } else {
            me->itsMemBlock[n16TmpBlock].sHeader.n8Status = BLOCK_STS_FREE;
        }
    
        /* Puts the last block obtained on the recombination process in the corresponding free list*/
        FreeList_add(&me->itsFreeList[n8TmpSize],n16TmpBlock);
        eReturnValue = Error_None;
    }
    
    return eReturnValue;
    
    /*#]*/
}

/*## operation combineBlocks(Ohb_UInt16_t,Ohb_UInt16_t) const */
static Ohb_UInt16_t combineBlocks(const MemPool* const me, Ohb_UInt16_t n16Block1, Ohb_UInt16_t n16Block2) {
    /*#[ operation combineBlocks(Ohb_UInt16_t,Ohb_UInt16_t) const */
    
    Ohb_UInt16_t n16Buddy0;
    Ohb_UInt16_t n16Buddy1;
    MemBlock_Size_t nNewSize;
    
    /* Identifies which is the "low" and "high" buddy */
    /* Note that nBlok1, and nBlock2 can point to the same MemBlock when combining two blocks of the smaller size. */
    if ( n16Block1 > n16Block2 )
    {
        n16Buddy0 = n16Block2;
        n16Buddy1 = n16Block1;
    }
    else
    {
        n16Buddy0 = n16Block1;
        n16Buddy1 = n16Block2;
    }
    
    nNewSize = me->itsMemBlock[n16Buddy0].sHeader.n8Size + 1u;
    /* Sets the Header to indicate their new status */
    me->itsMemBlock[n16Buddy0].sHeader.n8Size = nNewSize;
    me->itsMemBlock[n16Buddy1].sHeader.n8Size = nNewSize;
    me->itsMemBlock[n16Buddy0].sHeader.n8Status = BLOCK_STS_FREE;
    me->itsMemBlock[n16Buddy1].sHeader.n8Status = BLOCK_STS_FREE;
    
    return n16Buddy0;
    
    /*#]*/
}

/*## operation divideBlock(Ohb_UInt16_t) const */
static Ohb_UInt16_t divideBlock(const MemPool* const me, Ohb_UInt16_t n16Block) {
    /*#[ operation divideBlock(Ohb_UInt16_t) const */
    
    Ohb_UInt16_t n16Buddy;
    MemBlock_Size_t nNewSize;
    
    nNewSize = me->itsMemBlock[n16Block].sHeader.n8Size - 1u;
    
    me->itsMemBlock[n16Block].sHeader.n8Size = nNewSize;
    n16Buddy = getBuddy(me,n16Block);
    me->itsMemBlock[n16Buddy].sHeader.n8Size =nNewSize;
    me->itsMemBlock[n16Buddy].sHeader.n8Status = BLOCK_STS_FREE;
    
    return n16Buddy;
    
    
    /*#]*/
}

/*## operation getBuddy(Ohb_UInt16_t) const */
static Ohb_UInt16_t getBuddy(const MemPool* const me, Ohb_UInt16_t n16Block) {
    /*#[ operation getBuddy(Ohb_UInt16_t) const */
    
    /* The size of the block is decreased by one to find the bit that should be swapped to find the buddy */
    Ohb_Int16_t i16SwapCount = (Ohb_Int16_t)me->itsMemBlock[n16Block].sHeader.n8Size - 1;
    Ohb_UInt16_t n16Buddy = n16Block;
    
    
    if ( i16SwapCount >= 0 ){
    /* To find the buddy the bit in the "size" position shall be swapped */
    Ohb_UInt16_t n16SwapMask = (Ohb_UInt16_t)1u << i16SwapCount;
    n16Buddy = n16Block ^ n16SwapMask;
    }
    /* The buddy of the MemBlock of minimum size is itself */
    
    return n16Buddy;
    
    /*#]*/
}

/*## operation getAddress(Ohb_UInt16_t) const */
static Ohb_UInt8_t * getAddress(const MemPool* const me, Ohb_UInt16_t n16Block) {
    /*#[ operation getAddress(Ohb_UInt16_t) const */
    
    /* Computes the byte index multiplying the block index by the min block size.*/
    Ohb_UInt32_t nIndex =  (Ohb_UInt32_t)n16Block << me->n8KMin;
    
    return (&me->pData[nIndex]);
    
    /*#]*/
}

/*## operation initialise() */
static Ohb_Error_t initialise(MemPool* const me) {
    /*#[ operation initialise() */
    
    Ohb_Error_t  eReturnValue;
    Ohb_UInt8_t  n8Index;
    Ohb_UInt16_t n16Index;
    Ohb_UInt16_t n16NbBlocksOfMaxSize;
    Ohb_UInt16_t n16NbMinSizeBlocksInMaxSize;
    Ohb_UInt16_t n16BlockIndex;
    
    /* Initialise computed values */
    me->nMinBlockSize = ((Ohb_UInt32_t)1u << me->n8KMin);
    
    /* FreeLists initialisation */
    for (n8Index=0u; n8Index <= me->n8K; n8Index++)    {
    	FreeList_init(&me->itsFreeList[n8Index], me->itsMemBlock);
    }
    
    /* Initialises the block of maximum size and adds them to the max  size list (list itsFreeList[n8K]) */
    
    /* Computes the number of maximum size blocks*/
    n16NbBlocksOfMaxSize = (Ohb_UInt16_t)(me->nNbBytes / ((Ohb_UInt32_t)1u << me->n8KMax));
    
    /* Computes the number of min size blocks in a max size block*/
    n16NbMinSizeBlocksInMaxSize = (Ohb_UInt16_t)1u << me->n8K;   /*2^n8K*/
    
    if (n16NbBlocksOfMaxSize < 1u) {
    
    	/* Error if total memory pool size is less than one block of maximum size */
    	eReturnValue = Error_Failed;
    }
    /* The number of MemBlocks, shall be the total number of physical blocks of min size divided by 2 */
    else if ((Ohb_UInt32_t)me->n16NbBlocks != ((me->nNbBytes / me->nMinBlockSize) >> 1U)) {
    
    	/* Error if the number of MemBlocks is not coherent with the array size*/
    	eReturnValue = Error_Failed;
    }
    else {
    
    	eReturnValue = Error_None;
    
    	/* Adds the blocks of maximum size to the corresponding list */
    
    	for (n16Index = 0u; n16Index < n16NbBlocksOfMaxSize; n16Index++){
    		/* Computes the MemBlock index in array, is the actual block of memory divided by 2 */
    		n16BlockIndex = (n16Index * n16NbMinSizeBlocksInMaxSize) >> 1u;
    		me->itsMemBlock[n16BlockIndex].sHeader.n8Size = me->n8K;
    		me->itsMemBlock[n16BlockIndex].sHeader.n8Status = BLOCK_STS_FREE;
    		FreeList_add(&me->itsFreeList[me->n8K], n16BlockIndex);
    	}
    }
    
    return eReturnValue;
    /*#]*/
}
