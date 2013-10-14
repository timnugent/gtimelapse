/** @file gtimelapse.cpp
 *
 *  @author Tim Nugent <timnugent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "wx/wx.h"
#include "wx/imaglist.h"
#include <wx/dcbuffer.h>
#include <wx/filepicker.h>
#include "wx/thread.h"
#include "gtimelapse.h"
#include "gphotofunctions.h"
#include "icon.xpm"
#include "globals.h"

using namespace std;


MyThread::MyThread(MyFrame *frame) : wxThread(){
	m_count = 0;
	m_frame = frame;
}

void MyThread::OnExit()
{
    wxCriticalSectionLocker locker(wxGetApp().m_critsect);

    wxArrayThread& threads = wxGetApp().m_threads;
    threads.Remove(this);

    if ( threads.IsEmpty() )
    {
        // signal the main thread that there are no more threads left if it is
        // waiting for us
        if ( wxGetApp().m_waitingUntilAllDone )
        {
            wxGetApp().m_waitingUntilAllDone = false;

            wxGetApp().m_semAllDone.Post();
        }
    }
}

void *MyThread::Entry(){

	// Start timelapse capture here
	total_frames = 0;
	time_t start_time = time(0);
	string stlstring = string(working_direcrory->GetPath().mb_str());

	long interval_seconds, max_runtime_mins, max_frames_total; 

	if(!interval->GetValue().ToLong(&interval_seconds)) {cout << "Error getting interval" << endl;}	
	if(!max_runtime->GetValue().ToLong(&max_runtime_mins)) {cout << "Error getting runtime" << endl;}
	if(!max_frames->GetValue().ToLong(&max_frames_total)) {cout << "Error getting max frames" << endl;}

	start_capture = true;
	button3->Disable();
	while(start_capture){

		mutex_enter = true;
		wxMutexGuiEnter();
		this->m_frame->CaptureImage();
		wxMutexGuiLeave();		
		mutex_enter = false;
		
		if (start_capture) cout << "Sleeping for " << interval_seconds << " seconds... ";
		sleep(interval_seconds);
		if (start_capture) cout << "done." << endl << endl;
		
		if(max_frames_total != 0 && total_frames >= max_frames_total){
			cout << max_frames_total << " frames captured - stopping capture." << endl;
			start_capture = false;
		}
		if(max_runtime_mins != 0){
		
			time_t curr_time = time(0);			
			if ((int)((curr_time - start_time)/60) >= max_runtime_mins){
				cout << max_runtime_mins << " minutes have elapsed - stopping capture." << endl;
				start_capture = false;
			}
		}
	}
	if (start_capture) cout << endl;
	button2->Enable();
	button3->Enable();
	button4->Disable();
		
    	return NULL;
}

wxPanel *display_widgets (wxPanel *panel, CameraWidget *widget, char *prefix){

	int ret, n, i;
	char *newprefix;
	const char *label, *name, *uselabel;
	CameraWidgetType type;
	CameraWidget *rootconfig,*child;

	gp_widget_get_label (widget, &label);
	gp_widget_get_name (widget, &name);
	gp_widget_get_type (widget, &type);

	if (strlen(name)){
		uselabel = name;
	}else{
		uselabel = label;
	}

	n = gp_widget_count_children (widget);

	newprefix = new char[strlen(prefix)+1+strlen(uselabel)+1];
	
	if (!newprefix)
		abort();
	
	sprintf(newprefix,"%s/%s",prefix,uselabel);

	if ((type != GP_WIDGET_WINDOW) && (type != GP_WIDGET_SECTION))

		cout << "Found parameter: " << uselabel << endl;
		ret = find_widget_by_name (&gp_params, uselabel, &child, &rootconfig);
		ret = gp_widget_get_type (child, &type);		
		ret = gp_widget_get_label (child, &label);

		if (type == GP_WIDGET_RADIO){
			int cnt, i;
			char *current;

			ret = gp_widget_get_value (child, &current);
			if (ret == GP_OK) {
				cnt = gp_widget_count_choices (child);
				if (type == GP_WIDGET_MENU){
					//cout << "type: MENU" << endl;
				}else{
					//cout << "type: RADIO" << endl;
					//cout << "current: " << current << endl;

					wxString choices[cnt];
					
					for ( i=0; i<cnt; i++) {
						const char *choice;
						ret = gp_widget_get_choice (child, i, &choice);
						//cout << "choice: " << i << " " << choice << endl;
						wxString mystring(choice, wxConvUTF8);
						choices[i] = mystring;
					}
			
					wxString title(label, wxConvUTF8);
					wxString default_choice(current, wxConvUTF8);
					wxString choice_label(uselabel, wxConvUTF8);
					
					wxString mystring(uselabel, wxConvUTF8);
					choice_label_vector.push_back(mystring);					
					//cout << "label: " << label << endl;					

					// Generate comboboxes	
					new wxStaticText( panel, wxID_ANY, title, wxPoint(x, y+6), wxDefaultSize, wxALIGN_LEFT);
					wxComboBox *cb = new wxComboBox(panel, wxID_ANY, default_choice, wxPoint(250, y), wxSize(-1,-1), cnt, choices, wxALIGN_RIGHT, wxDefaultValidator, choice_label);
					y += 40;
					combobox_vector.push_back(cb);
				}
			}
		}		

	for (i=0; i<n; i++){	
		CameraWidget *child;	
		ret = gp_widget_get_child (widget, i, &child);
		if (ret != GP_OK){
			continue;
		}
		display_widgets (panel, child, newprefix);
	}
	free(newprefix);
	return(panel);
}

wxPanel *initialise_camera (wxPanel *panel){

	// Set up camera paramaters
	gp_params_init (&gp_params);
	
	// Set aside memory for camera
	int result = gp_camera_new (&gp_params.camera);
	//cout << "gp_camera_new:\t" << result << endl;  	
	
	// Initialise camera
	cout << "Detecting Camera." << endl << endl;
	result = gp_camera_init (gp_params.camera, gp_params.context);
	
	if (result != GP_OK) {	
	
		cout << "Failed to initialise camera:\t" << result << endl << gp_result_as_string (result) << endl;		
		
		if (result == -105){					
			wxLogError(wxT("Failed to initialise camera. Please check the camera is turned on, then re-initialise or restart the program to enable camera paramaters."));		
		}else if (result == -60){
			wxLogError(wxT("Failed to initialise camera. Please check the camera is unmounted and that no other applications are using it, then re-initialise or restart the program to enable camera paramaters."));			
		}
		//wxLogError(wxString::FromAscii(gp_result_as_string (result)));		
		button2->Disable();
		button3->Disable();
		button4->Disable();
		//panel->SetStatusText( _T("Failed to initialise camera.") );
	}else{
		cout << "Camera initialised." << endl << endl;
		//button1->Disable();
		button2->Enable();
		button3->Enable();
		button4->Disable();
		//panel->SetStatusText( _T("Camera initialised.") );

		CameraWidget *rootconfig;
		int ret = gp_camera_get_config (gp_params.camera, &rootconfig, gp_params.context);
		if (ret == GP_OK){
			char prefix[] = "";
			display_widgets (panel, rootconfig, prefix);
			gp_widget_free (rootconfig);
		}		
	}
	cout << endl;

	return(panel);
}	

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit(){

	// Add JPG handler
	wxImage::AddHandler( new wxJPEGHandler );

    	// Create the main window
    	MyFrame *frame = new MyFrame();
    	frame->SetSize(wxDefaultCoord, wxDefaultCoord, width, height);
	frame->SetMaxSize(wxSize(width, height));
    	frame->SetVirtualSize(wxSize(width, height));
	frame->Show();
 
    	return true;
        
}

wxPanel *CreateRadioButtonsPage(wxBookCtrlBase *parent){

    	wxPanel *panel = new wxPanel(parent);

	// Button to detect camera
	//button1 = new wxButton(panel, 10, wxT("Detect Camera"), wxPoint(20, 20));

	// Buttons to capture image, start/stop sequence, choose working dir
	button2 = new wxButton(panel, 20, wxT("Capture Image"), wxPoint(20, 20));
	button3 = new wxButton(panel, 40, wxT("Start"), wxPoint(130, 20));
	button4 = new wxButton(panel, 50, wxT("Stop"),  wxPoint(218, 20));
	new wxStaticText( panel, wxID_ANY, wxT("Working directory: "), wxPoint(320, 28), wxDefaultSize, wxALIGN_LEFT);	
	working_direcrory = new wxDirPickerCtrl(panel, 50, wxGetCwd(), wxT(""), wxPoint(445, 20), wxSize(200, 34), wxDIRP_CHANGE_DIR);
	
	// Other capture options
	new wxStaticText( panel, wxID_ANY, wxT("Interval: "), wxPoint(20, 70), wxDefaultSize, wxALIGN_LEFT);	
	interval = new wxTextCtrl(panel, -1, wxT("5"), wxPoint(178, 65), wxSize(60, wxDefaultCoord), 0, wxDefaultValidator, wxT("Interval"));
	new wxStaticText( panel, wxID_ANY, wxT("Seconds"), wxPoint(242, 70), wxDefaultSize, wxALIGN_LEFT);	    

	new wxStaticText( panel, wxID_ANY, wxT("Frames per interval: "), wxPoint(20, 100), wxDefaultSize, wxALIGN_LEFT);	
	frames_per_interval = new wxTextCtrl(panel, -1, wxT("1"), wxPoint(178, 95), wxSize(60, wxDefaultCoord), 0, wxDefaultValidator, wxT("Frames"));
	new wxStaticText( panel, wxID_ANY, wxT("Frames"), wxPoint(242, 100), wxDefaultSize, wxALIGN_LEFT);	

	new wxStaticText( panel, wxID_ANY, wxT("Maximum runtime: "), wxPoint(370, 70), wxDefaultSize, wxALIGN_LEFT);	
	max_runtime = new wxTextCtrl(panel, -1, wxT("0"), wxPoint(528, 65), wxSize(60, wxDefaultCoord), 0, wxDefaultValidator, wxT("Runtime"));
	new wxStaticText( panel, wxID_ANY, wxT("Minutes"), wxPoint(592, 70), wxDefaultSize, wxALIGN_LEFT);	    

	new wxStaticText( panel, wxID_ANY, wxT("Maximum frames: "), wxPoint(370, 100), wxDefaultSize, wxALIGN_LEFT);	
	max_frames = new wxTextCtrl(panel, -1, wxT("10"), wxPoint(528, 95), wxSize(60, wxDefaultCoord), 0, wxDefaultValidator, wxT("MaxFrames"));
	new wxStaticText( panel, wxID_ANY, wxT("Frames"), wxPoint(592, 100), wxDefaultSize, wxALIGN_LEFT);	

 	new wxStaticBox(panel, -1, wxT(""),  wxPoint(15, 125), wxSize(650, 502));

    	return panel;
    
}

wxPanel *CreateParamsPage(wxBookCtrlBase *parent){

   	//wxPanel *panel = new wxPanel(parent);
	//wxScrolledWindow *panel = new wxScrolledWindow(parent,wxID_ANY,wxDefaultPosition,wxSize(690,720),wxVSCROLL);
	wxScrolledWindow *panel = new wxScrolledWindow(parent);

	panel->EnableScrolling(false,true);
	panel->SetScrollbars(0, 10, 0, 100);

	//wxPanel *panel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(690,720));

	tb1 = new wxCheckBox(panel, wxID_ANY, wxT("Retrieve image from camera."), wxPoint(20, y), wxSize(-1,-1), 0, wxDefaultValidator, wxT("RetrieveImage"));
	tb1->SetValue(true);
	y += 40;	
	tb2 = new wxCheckBox(panel, wxID_ANY, wxT("Delete image from camera."), wxPoint(20, y), wxSize(-1,-1), 0, wxDefaultValidator, wxT("DeleteImage"));
	tb2->SetValue(true);
	y += 40;	
	
	initialise_camera(panel);
	
	//panel->FitInside();
	panel->SetClientSize(parent->GetSize());

    	return panel;
}

wxPanel *CreateImagessPage(wxBookCtrlBase *parent){

   	wxPanel *panel = new wxPanel(parent);

	//tb1 = new wxCheckBox(panel, wxID_ANY, wxT("Retrieve image from camera."), wxPoint(20, y), wxSize(-1,-1), 0, wxDefaultValidator, wxT("RetrieveImage"));
	//tb1->SetValue(true);
	//y += 40;	
	//tb2 = new wxCheckBox(panel, wxID_ANY, wxT("Delete image from camera."), wxPoint(20, y), wxSize(-1,-1), 0, wxDefaultValidator, wxT("DeleteImage"));
	//tb2->SetValue(true);
	//y += 40;	
	
	//initialise_camera(panel);

    	return panel;
}


void CreateInitialPages(wxBookCtrlBase *parent){

	// Create and add some panels to the notebook

	wxPanel *panel = CreateRadioButtonsPage(parent);
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(MyFrame::OnPaint));
	parent->AddPage( panel, wxT("Timelapse Settings"), false, -1 );

	panel = CreateParamsPage(parent);
	//panel->Fit();

	parent->AddPage( panel, wxT("Camera Settings"), false, -1 );

	panel = CreateImagessPage(parent);
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(MyFrame::OnPaintCapturedImages));
	parent->AddPage( panel, wxT("Captured Images"), false, -1 );

	parent->SetSelection(0);
}

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, wxString(wxT("gTimelapse"))){

    	m_type = Type_Notebook;

    	m_orient = ID_ORIENT_DEFAULT;

    	SetIcon(wxICON(icon));

    	// menu of the sample
    	wxMenu *menuType = new wxMenu;

    	menuType->Check(ID_BOOK_NOTEBOOK + m_type, true);

    	wxMenu *menuFile = new wxMenu;
    	menuFile->Append(wxID_EXIT, wxT("E&xit"), wxT("Quit the application"));
    	menuFile->Append(30, wxT("&Detect Camera"), wxT("Detect camera"));
    	wxMenuBar *menuBar = new wxMenuBar;
    	menuBar->Append(menuFile, wxT("&File"));
    	SetMenuBar(menuBar);

    	// books creation
    	m_panel    = NULL;
    	m_bookCtrl = NULL;

    	m_panel = new wxPanel(this);

    	// Set sizers
    	m_sizerFrame = new wxBoxSizer(wxVERTICAL);

    	RecreateBook();

    	m_panel->SetSizer(m_sizerFrame);
    	m_sizerFrame->Fit(this);
    	m_sizerFrame->SetSizeHints(this);

    	Centre(wxBOTH);
    
    	CreateStatusBar();
    
}

MyFrame::~MyFrame(){

    delete m_imageList;
}

void MyFrame::RecreateBook(){

    	m_bookCtrl = new wxNotebook(m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_TOP);

    	if ( !m_bookCtrl )
        	return;

    	m_bookCtrl->Hide();

    	CreateInitialPages(m_bookCtrl);

    	m_sizerFrame->Insert(0, m_bookCtrl, wxSizerFlags(5).Expand().Border());

    	m_sizerFrame->Show(m_bookCtrl);
    	m_sizerFrame->Layout();
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)

	// Paint event
	EVT_PAINT(MyFrame::OnPaint)
	
	// Combobox event
	EVT_COMBOBOX(wxID_ANY, MyFrame::OnComboBox)
	
    	// Book controls
    	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, MyFrame::OnNotebook)
    	EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY, MyFrame::OnNotebook)

	// Buttons
	EVT_BUTTON(10,MyFrame::DetectCamera)
	EVT_BUTTON(20,MyFrame::CaptureImageButtonPressed)
	EVT_BUTTON(40,MyFrame::StartCapture)
	EVT_BUTTON(50,MyFrame::StopCapture)
	
    	// File menu
    	EVT_MENU(wxID_EXIT, MyFrame::OnExit)
    	EVT_MENU(30, MyFrame::DetectCamera)

    	// Update title in idle time
    	EVT_IDLE(MyFrame::OnIdle)
  
END_EVENT_TABLE()

void MyFrame::OnExit(wxCommandEvent& WXUNUSED(event)){
	
	Close();
}

void MyFrame::OnIdle(wxIdleEvent& WXUNUSED(event)){

    	static int s_nPages = wxNOT_FOUND;
    	static int s_nSel = wxNOT_FOUND;
    	static wxBookCtrlBase *s_currBook = NULL;

    	wxBookCtrlBase *currBook = GetCurrentBook();

    	int nPages = currBook ? currBook->GetPageCount() : 0;
    	int nSel = currBook ? currBook->GetSelection() : wxNOT_FOUND;

    	if ( nPages != s_nPages || nSel != s_nSel || s_currBook != currBook ){
        	s_nPages = nPages;
        	s_nSel = nSel;
        	s_currBook = currBook;

        	wxString selection;
        	if ( nSel == wxNOT_FOUND ){
            		selection << wxT("not found");
        	}else{
            		selection << nSel;
		}
        	
		wxString title;
        	title.Printf(wxT("gTimelapse"));
        	SetTitle(title);
    	}
    		
	if (!mutex_enter)Refresh();

}

void MyFrame::OnBookCtrl(wxBookCtrlBaseEvent& event){

    	static const struct EventInfo{
        	wxEventType typeChanged,typeChanging;
        	const wxChar *name;
    	} events[] = {
        	{
            		wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
            		wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING,
            		_T("wxNotebook")
        	},
    	};

   	wxString nameEvent,nameControl,veto;
    	const wxEventType eventType = event.GetEventType();
    
	for (size_t n = 0; n < WXSIZEOF(events); n++){
        	const EventInfo& ei = events[n];
        	if (eventType == ei.typeChanged){
         	   	nameEvent = wxT("Changed");
        	}else{
        		continue;
        	}

        	nameControl = ei.name;
        	break;
    	}
}

void MyFrame::DetectCamera(wxCommandEvent& event){

	// Close connection to camera
	gp_camera_exit (gp_params.camera, gp_params.context);
	
	// Set up camera paramaters
	gp_params_init (&gp_params);
	
	// Set aside memory for camera
	int result = gp_camera_new (&gp_params.camera);
	//cout << "gp_camera_new:\t" << result << endl;  	
	
	// Initialise camera
	cout << "Detecting Camera." << endl << endl;
	result = gp_camera_init (gp_params.camera, gp_params.context);

	if (result != GP_OK) {		

		cout << "Failed to initialise camera:\t" << result << endl << gp_result_as_string (result) << endl;	

		if (result == -105){					
			wxLogError(wxT("Failed to initialise camera. Please check the camera is turned on, then re-initialise or restart the program to enable camera paramaters."));		
		}else if (result == -60){
			wxLogError(wxT("Failed to initialise camera. Please check the camera is unmounted and that no other applications are using it, then re-initialise or restart the program to enable camera paramaters."));			
		}

	}else{
		cout << "Camera initialised." << endl;
		button2->Enable();
		button3->Enable();
		this->SetStatusText( _T("Camera initialised.") );
	}
	cout << endl;
}

void MyFrame::CaptureImage(){

	// Set camera paramaters if changed
	if (params_changed){	
		cout << "Setting camera paramaters:" << endl;			
		for (unsigned int v = 0; v < choice_label_vector.size(); v++){
		
			cout << "Paramater:\t" << choice_label_vector[v].mb_str() << endl;
			cout << "Value:\t\t" << combobox_vector[v]->GetValue().mb_str() << endl;
			
			int set = set_config_action(&gp_params,choice_label_vector[v].mb_str(),combobox_vector[v]->GetValue().mb_str());
			if (set == GP_OK){
				//cout << "OK" <<  endl;
			}else{
				cout << "Error setting paramater:\t" << set << endl;
			}
		}	
		params_changed = false;
		cout << endl;
	}

	// Now take picture
	cout << "Capturing Image..." << endl;
	CameraFile * camera_file;
	CameraFilePath path;
	vector<string> camera_files;
	vector<string> camera_folders;
	
	int result = 0;
	long frames_per_int;
	if(!frames_per_interval->GetValue().ToLong(&frames_per_int)) {cout << "Error getting frames/interval" << endl;}
	
	for (int f = 0; f < frames_per_int; f++){

		result = gp_camera_capture (gp_params.camera, GP_CAPTURE_IMAGE, &path, gp_params.context);

		if (result != GP_OK) {
			cout << "Could not capture image:\t" << result << endl;
			cout << "Check the camera and re-initialise." << endl;
			wxLogError(wxT("Could not capture an image. Please check the camera and re-initialise."));
			start_capture = false;
			gp_camera_exit(gp_params.camera, gp_params.context);
			button2->Disable();
			button3->Disable();
			button4->Disable();
		}else{
			total_frames++;
			cout << "New file is in location " << path.folder << "/" << path.name << " on the camera." << endl;
			wxString status = (wxString::FromAscii("New file is in location "));
			status.Append(wxString::FromAscii(path.folder));
			status.Append(wxString::FromAscii("/"));
			status.Append(wxString::FromAscii(path.name));
			status.Append(wxString::FromAscii(" on the camera."));
			this->SetStatusText(status);
	
			camera_files.push_back(path.name);
			camera_folders.push_back(path.folder);
		}			
	}

	// Now get and delete images
	if (camera_files.size()){
	
		for (unsigned int f = 0; f < camera_files.size(); f++){

			string local_path = string(working_direcrory->GetPath().mb_str());
			local_path += "/";
			local_path += camera_files[f];
			gp_file_new (&camera_file);
		
			// Retrieve image if checkbox is ticked
			if (tb1->IsChecked()){		
		
				result = gp_camera_file_get (gp_params.camera, camera_folders[f].c_str(), camera_files[f].c_str(), GP_FILE_TYPE_NORMAL, camera_file, gp_params.context); 	
		
				if (result != GP_OK) {
					cout << "Could not retieve image:\t" << result << endl;
				}else{
					//result = gp_file_save (camera_file, path.name);
					result = gp_file_save (camera_file, local_path.c_str());
			
					if (result != GP_OK){
						cout << "Couldn't write file " << local_path << " - check path/permissions/disk space." << endl;
						wxString status = (wxString::FromAscii("Couldn't write file "));
						status.Append(wxString::FromAscii(local_path.c_str()));
						status.Append(wxString::FromAscii(" - check path/permissions/disk space."));		
						this->SetStatusText(status);	

					}else{
						if (start_capture){
							cout << "Written file " << total_frames << ": " << local_path << endl;							
							wxString status = (wxString::FromAscii("Written file "));
							status.Append(wxString::Format(_T("%d"), total_frames));
							status.Append(wxString::FromAscii(": "));
							status.Append(wxString::FromAscii(local_path.c_str()));
							this->SetStatusText(status);							
						}else{
							cout << "Written file " << local_path << endl;							
							wxString status = (wxString::FromAscii("Written file "));
							status.Append(wxString::FromAscii(local_path.c_str()));
							this->SetStatusText(status);						
						}
					}						
					
					// Delete image from camera if checkbox is ticked
					if (tb2->IsChecked()){	
				
						result = gp_camera_file_delete (gp_params.camera, camera_folders[f].c_str(), camera_files[f].c_str(), gp_params.context);
						if (result != GP_OK) {
							cout << "Problem deleting file from camera." << endl;
							wxString status = (wxString::FromAscii("Problem deleting file from camera."));
							this->SetStatusText(status);	
						}else{
							cout << "File " << camera_files[f] << " deleted from camera." << endl;
							gp_file_unref (camera_file);
						}					
					}
					cout << endl;
				}
			}
		}	
	}

	// Display the last image in the GUI
	if (result == GP_OK && tb1->IsChecked()){

		wxString name(path.name, wxConvUTF8);			
		wxBitmap bitmap;
		wxImage image = bitmap.ConvertToImage();			
		
		if ( !image.LoadFile(name, wxBITMAP_TYPE_ANY, -1) ){
			cout << "Couldn't load image." << endl;
    		}else{
			image.Rescale(resize_x, resize_y);
        		retrieved_image = wxBitmap(image);
			small_images.push_back(wxBitmap(image.Rescale(90,60)));
			if(small_images.size() > 48){
				small_images.erase(small_images.begin());
			}
		}	
	}
}

void MyFrame::CaptureImageButtonPressed(wxCommandEvent& event){

	MyFrame::CaptureImage();
}

void MyFrame::OnComboBox(wxCommandEvent& event){

	params_changed = true;
}

void MyFrame::OnPaint(wxPaintEvent& event){

	wxAutoBufferedPaintDC dc(this);
	PrepareDC( dc );  
	  	
	if (retrieved_image.Ok()){
		dc.DrawBitmap( retrieved_image, 20, 140);		
	}	
	event.Skip();
}

void MyFrame::OnPaintCapturedImages(wxPaintEvent& event){

	//cout << "OnPaintCapturedImages" << endl;
	wxAutoBufferedPaintDC dc(this);
	PrepareDC( dc );  
	
	unsigned int x_start = 20, y_start = 20;
	
	for (unsigned int i = 0; i < small_images.size(); i++){
	
		dc.DrawBitmap(small_images[i], x_start, y_start);
		x_start += 90 + 20;
		if (x_start >= 610){
			x_start = 20;
			y_start += 60 + 15;
		}
	}

	event.Skip();
}

void MyFrame::StartCapture(wxCommandEvent& event){

	cout << "Starting capture." << endl << endl;

	button2->Disable();
	button4->Enable();
	thread = CreateThread();

	if ( thread->Run() != wxTHREAD_NO_ERROR ){
		wxLogError(wxT("Can't start thread!"));
    	}else{
		//cout << "Thread started." << endl;
	}
}

void MyFrame::StopCapture(wxCommandEvent& event){

	cout << endl << "Stopping capture." << endl << endl;
	start_capture = false;
	mutex_enter = false;
	total_frames = 0;
	button2->Enable();
	button3->Enable();
	button4->Disable();

   	// stop the last thread
    	if (wxGetApp().m_threads.IsEmpty()){
	
        	wxLogError(wxT("No thread to stop!"));
        	wxGetApp().m_critsect.Leave();
		
    	}else{
        	
		wxThread *thread = wxGetApp().m_threads.Last();
        	// it's important to leave critical section before calling Delete()
        	// because delete will (implicitly) call OnExit() which also tries
        	// to enter the same crit section - would dead lock.
        	wxGetApp().m_critsect.Leave();
		//cout << "Deleting thread." << endl;
        	thread->Delete();
    	}
}

MyThread *MyFrame::CreateThread(){

	MyThread *thread = new MyThread(this);

	if ( thread->Create() != wxTHREAD_NO_ERROR ) {
		wxLogError(wxT("Can't create thread!"));
	}

	wxCriticalSectionLocker enter(wxGetApp().m_critsect);
	wxGetApp().m_threads.Add(thread);

    	return thread;
}
