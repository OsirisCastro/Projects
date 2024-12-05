#include "Window.h"
#include "wx/wx.h"

wxIMPLEMENT_APP(Window);

bool Window::OnInit() {

	mainWindow = new MainWindow();
	mainWindow->Show();
	return true;
}

