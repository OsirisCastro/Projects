#pragma once

#include "wx/wx.h"
#include "MainWindow.h"
#include "Setting.h"

class App : public wxApp
{
private:
	MainWindow* mainWindow;
	Setting settings;

public:
	App();
	~App();
	virtual bool OnInit();
};

