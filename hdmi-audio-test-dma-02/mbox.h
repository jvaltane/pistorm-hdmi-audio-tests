/*
    Copyright © 2021 Michal Schulz <michal.schulz@gmx.de>
    https://github.com/michalsc

    This Source Code Form is subject to the terms of the
    Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _MBOX_H
#define _MBOX_H

#include <stdint.h>

#include <exec/types.h>

/* param: mbox - read from devicetree */  
int mbox_init(APTR mbox);
void mbox_free(void);

/* ret: dma channel mask bits: 0-15. 0 - free 1 - used */  
uint32_t mbox_dma_mask_get(void);

#endif /* _MBOX_H */
