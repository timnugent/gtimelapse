#ifndef TIMELAPSE_H
#define TIMELAPSE_H

#include "wx/notebook.h"

WX_DEFINE_ARRAY_PTR(wxThread *, wxArrayThread);

// Define a new application
class MyApp : public wxApp {
	
	public:
	
    		bool OnInit();

    		// all the threads currently alive - as soon as the thread terminates, it's
    		// removed from the array
    		wxArrayThread m_threads;

    		// crit section protects access to all of the arrays below
    		wxCriticalSection m_critsect;

    		// semaphore used to wait for the threads to exit, see MyFrame::OnQuit()
    		wxSemaphore m_semAllDone;

    		// the last exiting thread should post to m_semAllDone if this is true
    		// (protected by the same m_critsect)
    		bool m_waitingUntilAllDone;

};

DECLARE_APP(MyApp)

class MyThread;
class MyFrame;

class MyThread : public wxThread {

	public:
	
    		MyThread(MyFrame *frame);

    		// thread execution starts here
    		virtual void *Entry();

    		// called when the thread exits - whether it terminates normally or is
    		// stopped with Delete() (but not when it is Kill()ed!)
    		virtual void OnExit();

    		// write something to the text control
    		void WriteText(const wxString& text);

    		unsigned m_count;
    		MyFrame *m_frame;
};


class MyFrame : public wxFrame {

	public:
    		
		MyFrame();
    		virtual ~MyFrame();

    		void OnExit(wxCommandEvent& event);
    		void OnBookCtrl(wxBookCtrlBaseEvent& event);
    		void OnNotebook(wxNotebookEvent& event) { OnBookCtrl(event); }
    		void OnIdle(wxIdleEvent& event);
		void DetectCamera(wxCommandEvent& event);
		void CaptureImageButtonPressed(wxCommandEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnPaintCapturedImages(wxPaintEvent& event);
		void OnComboBox(wxCommandEvent& event);
		void StartCapture(wxCommandEvent& event);
		void StopCapture(wxCommandEvent& event);
		void CaptureImage();
		
    		wxBookCtrlBase *GetCurrentBook() const { return m_bookCtrl; }

	private:

    		size_t m_nRunning, m_nCount;

    		wxLog *m_logTargetOld;
    		MyThread *CreateThread();

    		void RecreateBook();
    		wxPanel *CreateNewPage() const;
    		int TranslateBookFlag(int nb, int lb, int chb, int tbk, int toolbk) const;
    		void AddFlagStrIfFlagPresent(wxString & flagStr, long flags, long flag, const wxChar * flagName) const;

    		// Sample setup
    		enum BookType{
        		Type_Notebook,
		        Type_Max
    		} m_type;
    		int m_orient;

    		// Controls

    		wxPanel *m_panel; // Panel containing notebook and other controls
    		wxBookCtrlBase *m_bookCtrl;

    		wxBoxSizer *m_sizerFrame;

    		wxImageList *m_imageList;

    		DECLARE_EVENT_TABLE()
};

enum ID_COMMANDS {

    // these should be in the same order as Type_XXX elements above
    ID_BOOK_NOTEBOOK = wxID_HIGHEST,
    ID_BOOK_MAX,
    ID_ORIENT_DEFAULT,
    ID_ORIENT_TOP,
    ID_ORIENT_BOTTOM,
    ID_ORIENT_LEFT,
    ID_ORIENT_RIGHT,
    ID_ORIENT_MAX,
    ID_SHOW_IMAGES,
    ID_MULTI,
    ID_ADD_PAGE,
    ID_ADD_PAGE_NO_SELECT,
    ID_INSERT_PAGE,
    ID_DELETE_CUR_PAGE,
    ID_DELETE_LAST_PAGE,
    ID_NEXT_PAGE,
    ID_ADD_PAGE_BEFORE,
    ID_ADD_SUB_PAGE,
    ID_GO_HOME,
    ID_HITTEST
    
};

#endif
