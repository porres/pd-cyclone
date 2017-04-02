// for bitwise classes

int32_t bitwise_getbitmask(int ac, t_atom *av);

typedef  union
{
    t_float f;
    unsigned int ui;
}t_isdenorm;

static inline int BITWISE_ISDENORM(t_float f)
{
	t_isdenorm mask;
	mask.f = f;
	return ((mask.ui & 0x07f800000) == 0);
}
