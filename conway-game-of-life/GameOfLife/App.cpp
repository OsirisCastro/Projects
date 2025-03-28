#include "App.h"
#include "MainWindow.h"
#include "Setting.h"
#include "SettingsDialog.h"

wxIMPLEMENT_APP(App);

App::App() {

}

App::~App() {

}

bool App::OnInit() {
    settings.LoadSettingsFromFile();
    settings = Setting();

    mainWindow = new MainWindow();
    mainWindow->Show();

    return true;
}