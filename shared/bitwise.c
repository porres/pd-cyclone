#include "m_pd.h"
#include "g_canvas.h" // needed?
#include "shared.h" // needed?
#include "bitwise.h"

int32_t bitwise_getbitmask(int ac, t_atom *av)
{
    int32_t result = 0;
    if (sizeof(shared_t_bitmask) >= sizeof(int32_t))
    {
        int32_t nbits = sizeof(int32_t) * 8;
        shared_t_bitmask bitmask = 1 << (nbits - 1);
        if (ac > nbits)
            ac = nbits;
        while (ac--)
            {
            if (av->a_type == A_FLOAT &&
            (int)av->a_w.w_float)  /* CHECKED */
		    result += 1 << ac;
            av++;
            }
        }
    else bug("sizeof(shared_t_bitmask)");
    return (result);
}
