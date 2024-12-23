#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <exec/execbase.h>
#include <clib/debug_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>
#include <stdint.h>

#include "dma.h"
#include "common.h"
#include "support.h"


/* reason for simplicity: only one dma control block in use */
static uint8_t *_dma_pool = NULL;
static uint8_t *_dma_pool_ptr = NULL;
#define MAX_DCB_COUNT (3)
#define MAX_BUF_COUNT (1)
#define PADDING_SIZE (32)

int dma_init(void)
{
    /* enough for twp dcb structs + two buffers (src and dst) */
    _dma_pool = (uint8_t *)AllocVec(MAX_DCB_COUNT * sizeof(struct dcb) +
        MAX_BUF_COUNT * BUF_SIZE +
        (MAX_DCB_COUNT + MAX_BUF_COUNT) * PADDING_SIZE,
        MEMF_FAST|MEMF_CLEAR);
    if (_dma_pool == NULL) return 1;
    /* make it 32 byte aligned */
    _dma_pool_ptr = (uint8_t *)(((uintptr_t)_dma_pool + 31) & ~(uintptr_t)0x1F);
    if (_dma_pool_ptr == NULL) return 1;
    return 0;
}

void dma_free (void)
{
    if (_dma_pool) FreeVec(_dma_pool);
    _dma_pool = NULL;
    _dma_pool_ptr = NULL;
}


APTR dma_get_mem(size_t size)
{
    if (_dma_pool == NULL) return NULL;
    else if (_dma_pool_ptr == NULL) return NULL;
    APTR ret = (APTR)_dma_pool_ptr;
    /* prepare for next get */
    _dma_pool_ptr = (uint8_t *)((uintptr_t)_dma_pool_ptr + (uintptr_t)size);
    /* make it 32 byte aligned again */
    _dma_pool_ptr = (uint8_t *)(((uintptr_t)_dma_pool_ptr + 31) & ~(uintptr_t)0x1F);
    return ret;
}

int dma_reset (uint32_t ch)
{
    if (ch > 14) return 1;

    /* Reset the DMA channel */
    *DMA_CH_CS(ch) = LE32(DMA_CHANNEL_ABORT);
    *DMA_CH_CS(ch) = 0;
    *DMA_CH_CS(ch) = LE32(DMA_CHANNEL_RESET);
    *DMA_CH_CB_ADDR(ch) = 0;
    *DMA_CH_CS(ch) = LE32(DMA_END_FLAG);

    return 0;
}

void dma_setup (struct dcb *cb, uint32_t src, uint32_t dst, uint32_t size)
{
    cb->ti  = LE32(DMA_TI_SRC_INC | DMA_TI_DST_DREQ | DMA_TI_PERMAP(DMA_TI_PERMAP_HDMI) | DMA_TI_BURST(2));
    cb->src = LE32((uint32_t)src);
    cb->dst = LE32((uint32_t)dst);
    cb->len = LE32(size);
}

void dma_start (uint32_t ch, struct dcb *cb)
{
    dma_reset(ch);

    *DMA_CH_CB_ADDR(ch) = LE32((uint32_t)cb);
    //*DMA_CH_CS(ch) = LE32(DMA_PRIORITY(8) | DMA_PANIC_PRIORITY(8)) // | DMA_DISDEBUG);
    *DMA_CH_CS(ch) = LE32(DMA_WAIT_ON_WRITES);
    *DMA_CH_CS(ch) |= LE32(DMA_ACTIVE);
    //*DMA_CH_CS(ch) = LE32(DMA_ACTIVE);
}

void dma_wait (uint32_t ch)
{
    while (LE32(*DMA_CH_CS(ch)) & 1) { Delay(10); }
}

