// from old fragile (bits and pieces likely to break with any new Pd version.)

t_outconnect *magic_outlet_connections(t_outlet *o);
t_outconnect *magic_outlet_nextconnection(t_outconnect *last, t_object **destp, int *innop);

