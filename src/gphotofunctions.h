#ifndef GPHOTOFUNCTIONS_H
#define GPHOTOFUNCTIONS_H

#include <gphoto2/gphoto2.h>

typedef struct _GPParams GPParams;
struct _GPParams {

	Camera *camera;
	GPContext *context;
	char *folder;
	char *filename;
	CameraAbilitiesList *_abilities_list;
	GPPortInfoList *portinfo_list;
	int debug_func_id;
};

void gp_params_init (GPParams *p);
int find_widget_by_name (GPParams *p, const char *, CameraWidget **, CameraWidget **);
int set_config_action (GPParams *p,const char *name, const char *value);

#endif
