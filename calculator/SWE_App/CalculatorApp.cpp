#include "CalculatorApp.h"
#include "MainWindow.h"

wxIMPLEMENT_APP(CalculatorApp);

bool CalculatorApp::OnInit() {

	MainWindow* mainWindow = new MainWindow(wxT("Calculator"));
	mainWindow->Show(true);

	return true;
}

