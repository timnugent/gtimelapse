#ifndef GLOBALS_H
#define GLOBALS_H

#include "gphotofunctions.h"
#include "wx/wx.h"
#include <vector>
#include <wx/filepicker.h>
#include "wx/thread.h"
#include "gtimelapse.h"

extern GPParams gp_params;
extern wxButton *button1, *button2, *button3, *button4, *button5;
extern unsigned int resize_x, resize_y, x, y;
extern int total_frames, width, height;
extern bool paint, params_changed, start_capture, mutex_enter;
extern wxBitmap retrieved_image;
extern wxCheckBox *tb1, *tb2;
extern wxTextCtrl *interval, *frames_per_interval, *max_runtime, *max_frames;
extern wxDirPickerCtrl *working_direcrory;
extern std::vector<wxString> choice_label_vector;
extern std::vector<wxComboBox*> combobox_vector;
extern std::vector<wxBitmap> small_images;
extern MyThread *thread;

#endif
