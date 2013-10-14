#include "globals.h"
#include "gphotofunctions.h"
#include <gphoto2/gphoto2.h>
#include <vector>
#include "wx/wx.h"
#include <wx/filepicker.h>
#include "wx/thread.h"
#include "gtimelapse.h"

GPParams gp_params;
wxButton *button1, *button2, *button3, *button4, *button5;

// Dimentsion of main window
int width = 690, height = 720;

// Resize and thumbnail dimension
unsigned int resize_x = 640, resize_y = 480, x = 20, y = 20;

int total_frames = 0;
bool paint = false, params_changed = false, start_capture = false, mutex_enter = false;
wxBitmap retrieved_image;
wxCheckBox *tb1, *tb2;
wxTextCtrl *interval, *frames_per_interval, *max_runtime, *max_frames;
wxDirPickerCtrl *working_direcrory;
std::vector<wxString> choice_label_vector;
std::vector<wxComboBox*> combobox_vector;
std::vector<wxBitmap> small_images;
MyThread *thread;
