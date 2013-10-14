#include <iostream>
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include "gphotofunctions.h"

void gp_params_init (GPParams *p){

	if (!p){
		return;
	}

	memset (p, 0, sizeof (GPParams));
	p->folder = strdup ("/");
	gp_camera_new (&p->camera);
	p->context = gp_context_new();
	p->_abilities_list = NULL;
}

int find_widget_by_name (GPParams *p, const char *name, CameraWidget **child, CameraWidget **rootconfig) {
	
	int ret;
	ret = gp_camera_get_config (p->camera, rootconfig, p->context);
	if (ret != GP_OK) return ret;
	ret = gp_widget_get_child_by_name (*rootconfig, name, child);
	if (ret != GP_OK) 
		ret = gp_widget_get_child_by_label (*rootconfig, name, child);
	if (ret != GP_OK) {
		char *part, *s, *newname;

		newname = strdup (name);
		if (!newname)
			return GP_ERROR_NO_MEMORY;

		*child = *rootconfig;
		part = newname;
		while (part[0] == '/')
			part++;
		while (1) {
			CameraWidget *tmp;

			s = strchr (part,'/');
			if (s)
				*s='\0';
			ret = gp_widget_get_child_by_name (*child, part, &tmp);
			if (ret != GP_OK)
				ret = gp_widget_get_child_by_label (*child, part, &tmp);
			if (ret != GP_OK)
				break;
			*child = tmp;
			if (!s) /* end of path */
				break;
			part = s+1;
			while (part[0] == '/')
				part++;
		}
		if (s) { /* if we have stuff left over, we failed */
			//gp_context_error (p->context, _("%s not found in configuration tree."), newname);
			free (newname);
			gp_widget_free (*rootconfig);
			return GP_ERROR;
		}
		free (newname);
	}
	return GP_OK;
}

int set_config_action (GPParams *p, const char *name, const char *value){
	
	CameraWidget *rootconfig,*child;
	int ret;
	const char *label;
	CameraWidgetType type;

	ret = find_widget_by_name (p, name, &child, &rootconfig);
	if (ret != GP_OK)
		return ret;

	ret = gp_widget_get_type (child, &type);
	if (ret != GP_OK) {
		gp_widget_free (rootconfig);
		return ret;
	}
	ret = gp_widget_get_label (child, &label);
	if (ret != GP_OK) {
		gp_widget_free (rootconfig);
		return ret;
	}

	switch (type) {
	case GP_WIDGET_TOGGLE: {
	}
	case GP_WIDGET_TEXT: {		/* char *		*/
		ret = gp_widget_set_value (child, value);
		if (ret != GP_OK)
			//gp_context_error (p->context, _("Failed to set the value of text widget %s to %s."), name, value);
		break;
	}
	case GP_WIDGET_RANGE: {	/* float		*/
		float	f,t,b,s;

		ret = gp_widget_get_range (child, &b, &t, &s);
		if (ret != GP_OK)
			break;
		if (!sscanf (value, "%f", &f)) {
			//gp_context_error (p->context, _("The passed value %s is not a floating point value."), value);
			ret = GP_ERROR_BAD_PARAMETERS;
			break;
		}
		if ((f < b) || (f > t)) {
			//gp_context_error (p->context, _("The passed value %f is not within the expected range %f - %f."), f, b, t);
			ret = GP_ERROR_BAD_PARAMETERS;
			break;
		}
		ret = gp_widget_set_value (child, &f);
		if (ret != GP_OK)
			//gp_context_error (p->context, _("Failed to set the value of range widget %s to %f."), name, f);
		break;
	}
	case GP_WIDGET_DATE:  {		/* int			*/
		int t = -1;
		if (t == -1) {
			if (!sscanf (value, "%d", &t)) {
				//gp_context_error (p->context, _("The passed value %s is neither a valid time nor an integer."), value);
				ret = GP_ERROR_BAD_PARAMETERS;
				break;
			}
		}
		ret = gp_widget_set_value (child, &t);
		if (ret != GP_OK)
			//gp_context_error (p->context, _("Failed to set new time of date/time widget %s to %s."), name, value);
		break;
	}
	case GP_WIDGET_MENU:
	case GP_WIDGET_RADIO: { /* char *		*/
		int cnt, i;

		cnt = gp_widget_count_choices (child);
		if (cnt < GP_OK) {
			ret = cnt;
			break;
		}
		ret = GP_ERROR_BAD_PARAMETERS;
		for ( i=0; i<cnt; i++) {
			const char *choice;

			ret = gp_widget_get_choice (child, i, &choice);
			if (ret != GP_OK)
				continue;
			if (!strcmp (choice, value)) {
				ret = gp_widget_set_value (child, value);
				break;
			}
		}
		if (i != cnt)
			break;

		if (sscanf (value, "%d", &i)) {
			if ((i>= 0) && (i < cnt)) {
				const char *choice;

				ret = gp_widget_get_choice (child, i, &choice);
				if (ret == GP_OK)
					ret = gp_widget_set_value (child, choice);
				break;
			}
		}
		//gp_context_error (p->context, _("Choice %s not found within list of choices."), value);
		break;
	}

	/* ignore: */
	case GP_WIDGET_WINDOW:
	case GP_WIDGET_SECTION:
	case GP_WIDGET_BUTTON:
		//gp_context_error (p->context, _("The %s widget is not configurable."), name);
		ret = GP_ERROR_BAD_PARAMETERS;
		break;
	}
	if (ret == GP_OK) {
		ret = gp_camera_set_config (p->camera, rootconfig, p->context);
		//if (ret != GP_OK)
			//gp_context_error (p->context, _("Failed to set new configuration value %s for configuration entry %s."), value, name);
	}
	gp_widget_free (rootconfig);
	return (ret);
}
	
