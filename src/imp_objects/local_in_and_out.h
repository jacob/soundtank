/*Copyright 2003 Jacob Robbins*/

#ifndef LOCAL_IN_AND_OUT_INCLUDE
#define LOCAL_IN_AND_OUT_INCLUDE

/*note: local_in has no implementation object*/
/*this is signified by RTOBJECT_IMP_TYPE_INLINE*/
/*functionality for local_out only uses buffers*/

typedef struct local_out local_out_t;
struct local_out{

  char *target;

};


local_out_t* imp_object_local_out_alloca();
void imp_object_local_out_dealloca(local_out_t* loc_out);


int create_imp_object_local_in(rtobject_t* rtobj);
int destroy_imp_object_local_in(rtobject_t* rtobj);

int create_imp_object_local_out(rtobject_t* rtobj);
int destroy_imp_object_local_out(rtobject_t* rtobj);


int init_instance_local_in(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_local_in(rtobject_t* rtobj, rtobject_instance_t* rtins);

int init_instance_local_out(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_local_out(rtobject_t* rtobj, rtobject_instance_t* rtins);


int local_out_attach(rtobject_t* rtobj, const char *target);




#endif
