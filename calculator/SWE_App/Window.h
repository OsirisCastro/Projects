#pragma once

#include "MainWindow.h"
#include "wx/wx.h"

class Window : public wxApp {
	MainWindow* mainWindow;

public:
	virtual bool OnInit();

};

