/*
    Copyright © 2021 Michal Schulz <michal.schulz@gmx.de>
    https://github.com/michalsc

    This Source Code Form is subject to the terms of the
    Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
#include <string.h>

#include <exec/types.h>
#include <proto/devicetree.h>

#include "mbox.h"
#include "support.h"

/* mbox */

#define MBOX_SIZE (512 * 4)

/* status register flags */

#define MBOX_TX_FULL (1UL << 31)
#define MBOX_RX_EMPTY (1UL << 30)
#define MBOX_CHANMASK 0xF

static APTR _mbox = 0;
static APTR _requestBase = 0;
static ULONG *_request = NULL;

static uint32_t mbox_recv(uint32_t channel)
{
    volatile uint32_t *mbox_read = (uint32_t*)(_mbox);
    volatile uint32_t *mbox_status = (uint32_t*)((uintptr_t)_mbox + 0x18);
    uint32_t response, status;

    do
    {
        do
        {
            status = LE32(*mbox_status);
            asm volatile("nop");
        }
        while (status & MBOX_RX_EMPTY);

        asm volatile("nop");
        response = LE32(*mbox_read);
        asm volatile("nop");
    }
    while ((response & MBOX_CHANMASK) != channel);

    return (response & ~MBOX_CHANMASK);
}

static void mbox_send(uint32_t channel, uint32_t data)
{
    volatile uint32_t *mbox_write = (uint32_t*)((uintptr_t)_mbox + 0x20);
    volatile uint32_t *mbox_status = (uint32_t*)((uintptr_t)_mbox + 0x18);
    uint32_t status;

    data &= ~MBOX_CHANMASK;
    data |= channel & MBOX_CHANMASK;

    do
    {
        status = LE32(*mbox_status);
        asm volatile("nop");
    }
    while (status & MBOX_TX_FULL);

    asm volatile("nop");
    *mbox_write = LE32(data);
}

int mbox_init(APTR mbox)
{
    /* get mail box */
    if (mbox == 0) {
        fprintf (stderr, "mbox = 0\n");
        return 1;
    }
    _mbox = mbox;
    /* Alloc 128-byte aligned memory for mailbox requests */
    _requestBase = AllocMem(MBOX_SIZE, MEMF_FAST);
    if (_requestBase == NULL) {
        fprintf (stderr, "Could not alloc mem for mbox request\n");
        return 1;
    }
    _request = (ULONG *)(((intptr_t)_requestBase + 127) & ~127);
    return 0;
}

void mbox_free(void)
{
    if (_requestBase != 0) FreeMem(_requestBase, MBOX_SIZE);
}


uint32_t mbox_dma_mask_get(void)
{
    ULONG *FBReq = _request;

    ULONG len = 8*4;
    FBReq[0] = LE32(8*4);
    FBReq[1] = 0;
    FBReq[2] = LE32(0x00060001); // GetDMAChannels
    FBReq[3] = LE32(4);
    FBReq[4] = 0;
    FBReq[5] = 0;
    FBReq[6] = 0;
    FBReq[7] = 0;

    CachePreDMA(FBReq, &len, 0);
    mbox_send(8, (ULONG)FBReq);
    mbox_recv(8);
    CachePostDMA(FBReq, &len, 0);

    for (int i = 4; i < 8; i++) {
        ULONG v = FBReq[i];
        v = LE32(v);
        printf ("state: LE32(FBReq[%02d]:0x%08x == %u\n", i, v, v);
    }

    return LE32(FBReq[5]); /* DMA channel-mask ???? */
}

