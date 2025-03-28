#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>
#include "Setting.h"

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent, Setting* settings);
    ~SettingsDialog();

private:
    Setting* settings;

    wxSpinCtrl* gridSizeSpinner = nullptr;
    wxColourPickerCtrl* livingCellColorPicker = nullptr;
    wxColourPickerCtrl* deadCellColorPicker = nullptr;

    void CreateControls();

    void OnOK(wxCommandEvent& event);

    void OnCancel(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
