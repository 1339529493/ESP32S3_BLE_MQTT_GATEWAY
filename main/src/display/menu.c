#include "smf.h"
#include "ui.h"

void menu_entry(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_center(label);
}

enum smf_state_result menu_run(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;
    return 0;
}

void menu_exit(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;
}