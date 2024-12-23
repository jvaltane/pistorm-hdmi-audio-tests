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
#define __NOLIBBASE__
#include <proto/devicetree.h>
#undef __NOLIBBASE__

#include "devicetree.h"

struct DeviceTree * DeviceTreeBase = NULL;
/*
    Some properties, like e.g. #size-cells, are not always available in a key, but in that case the properties
    should be searched for in the parent. The process repeats recursively until either root key is found
    or the property is found, whichever occurs first
*/
static CONST_APTR GetPropValueRecursive(APTR key, CONST_STRPTR property, APTR DeviceTreeBase)
{
    do {
        /* Find the property first */
        APTR prop = DT_FindProperty(key, property);

        if (prop)
        {
            /* If property is found, get its value and exit */
            return DT_GetPropValue(prop);
        }
        /* Property was not found, go to the parent and repeat */
        key = DT_GetParent(key);
    } while (key);

    return NULL;
}

int devicetree_init(void)
{
    DeviceTreeBase = (APTR)OpenResource((STRPTR)"devicetree.resource");
    if (DeviceTreeBase == NULL) {
        fprintf (stderr, "Could not open devicetree.resource\n");
        return 1;
    }
    return 0;
}

APTR devicetree_mbox_get(void)
{
    APTR key;
    APTR mbox = 0;
    /* Open device tree resource */
    key = DT_OpenKey("/aliases");
    if (key)
    {
        CONST_STRPTR mbox_alias = DT_GetPropValue(DT_FindProperty(key, "mailbox"));

        DT_CloseKey(key);

        if (mbox_alias != NULL)
        {
            key = DT_OpenKey(mbox_alias);

            if (key)
            {
                int size_cells = 1;
                int address_cells = 1;

                const ULONG * siz = GetPropValueRecursive(key, "#size_cells", DeviceTreeBase);
                const ULONG * addr = GetPropValueRecursive(key, "#address-cells", DeviceTreeBase);

                if (siz != NULL)
                    size_cells = *siz;

                if (addr != NULL)
                    address_cells = *addr;

                const ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "reg"));

                mbox = (APTR)reg[address_cells - 1];
                printf ("mbox0: 0x%08x\n", mbox);
                DT_CloseKey(key);
            }
        }
    }
    key = DT_OpenKey("/soc");
    if (key)
    {
        int size_cells = 1;
        int address_cells = 1;
        int cpu_address_cells = 1;

        const ULONG * siz = GetPropValueRecursive(key, "#size_cells", DeviceTreeBase);
        const ULONG * addr = GetPropValueRecursive(key, "#address-cells", DeviceTreeBase);
        const ULONG * cpu_addr = DT_GetPropValue(DT_FindProperty(DT_OpenKey("/"), "#address-cells"));

        if (siz != NULL)
            size_cells = *siz;

        if (addr != NULL)
            address_cells = *addr;

        if (cpu_addr != NULL)
            cpu_address_cells = *cpu_addr;

        const ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "ranges"));

        ULONG phys_vc4 = reg[address_cells - 1];
        ULONG phys_cpu = reg[address_cells + cpu_address_cells - 1];

        mbox = (APTR)((ULONG)mbox - phys_vc4 + phys_cpu);
        printf ("mbox1: 0x%08x\n", mbox);

        DT_CloseKey(key);
    }
    printf ("mbox : %p\n", (int *)mbox);
    return mbox;
}

