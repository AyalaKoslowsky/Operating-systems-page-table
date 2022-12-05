#include <stdio.h>
#include "os.h"


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);
uint64_t page_table_query(uint64_t pt, uint64_t vpn);


/* mapping vpn to ppn or destroying the vpn if ppn==NO_MAPPING */
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t nine_bytes_in_hex = 0x1FF;
    uint64_t curr_layer = vpn;
    uint64_t tmp;
    uint64_t layers_array[5] = {[0 ... 4] = 0x0};

    int i;
    for (i=0; i<5; i++)
    {
        tmp = curr_layer & nine_bytes_in_hex;
        layers_array[4-i] = tmp;
        curr_layer = curr_layer >> 9;
    }

    /* checking if the vpn exists */
    uint64_t *PTE = phys_to_virt(pt << 12);

    int layer;

    for (i=0; i<5; i++)
    {
        layer = layers_array[i];

        if ((i != 4) && ((PTE[layer] & 1) == 0)) /* vpn doesn't exist, and it's not the last layer */
        {
            if (ppn != NO_MAPPING) /* mapping vpn to ppn */
            {
                PTE[layer] = alloc_page_frame() << 12;
                PTE[layer]++; /*changing valid bit to 1 */
            }
            else /* destroying the vpn if it doesn't exists */
            {
                return;
            }
        }

        if (i==4) /* it's the last layer */
        {
            if (ppn != NO_MAPPING) /* mapping last layer */
            {
                PTE[layer] = ppn << 12;
                PTE[layer]++; /* changing valid bit to 1 */
                return;
            }
            else /* destroying the mapping */
            {
                PTE[layer] = 0;
                return;
            }
        }
        PTE = phys_to_virt(PTE[layer]-1); /* vpn exists, PTE becomes next layer */
    }
}

/* finding the mapping of a vpn in a pt */
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t nine_bytes_in_hex = 0x1FF;
    uint64_t curr_layer = vpn;
    uint64_t tmp;
    uint64_t layers_array[5] = {[0 ... 4] = 0x0};

    int i;
    for (i=0; i<5; i++)
    {
        tmp = curr_layer & nine_bytes_in_hex;
        layers_array[4-i] = tmp;
        curr_layer = curr_layer >> 9;
    }

    uint64_t *PTE = phys_to_virt(pt << 12);
    int layer;

    for (i=0; i<5; i++)
    {
        layer = layers_array[i];

        if ((PTE[layer] & 1) == 0) /* valid bit is 0 */
        {
            return NO_MAPPING;
        }
        if (i!=4)
        {
            PTE = phys_to_virt(PTE[layer]-1);
        }
    }

    return PTE[layer]>>12;
}
