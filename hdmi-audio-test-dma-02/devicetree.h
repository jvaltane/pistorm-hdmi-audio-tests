#ifndef _DMA_SIMPLE_DEVICETEE_H
#define _DMA_SIMPLE_DEVICETEE_H
/*
    Copyright © 2021 Michal Schulz <michal.schulz@gmx.de>
    https://github.com/michalsc

    This Source Code Form is subject to the terms of the
    Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <exec/types.h>

int devicetree_init(void);
APTR devicetree_mbox_get(void);

#endif /* _DMA_SIMPLE_DEVICETEE_H */
