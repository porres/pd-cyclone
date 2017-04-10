// for bitwise classes

int32_t bitwise_getbitmask(int ac, t_atom *av);

typedef union _i32_fl {
	int32_t if_int32;
	t_float if_float;
} t_i32_fl;

typedef  union _isdenorm {
    t_float f;
    uint32_t ui;
}t_isdenorm;

static inline int BITWISE_ISDENORM(t_float f)
{
	t_isdenorm mask;
	mask.f = f;
	return ((mask.ui & 0x07f800000) == 0);
}
