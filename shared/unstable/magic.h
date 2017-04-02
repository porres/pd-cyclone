// from old fragile (bits and pieces likely to break with any new Pd version.)

t_outconnect *magic_outlet_connections(t_outlet *o);
t_outconnect *magic_outlet_nextconnection(t_outconnect *last, t_object **destp, int *innop);

// from forky
int forky_hasfeeders(t_object *x, t_glist *glist, int inno, t_symbol *outsym);

