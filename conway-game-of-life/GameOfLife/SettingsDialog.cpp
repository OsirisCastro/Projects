#include "SettingsDialog.h"

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, SettingsDialog::OnCancel)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent, Setting* settings)
    : wxDialog(parent, wxID_ANY, "Settings"), settings(settings) {
    CreateControls();
    settings->LoadSettingsFromFile();
    *settings = Setting();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::CreateControls() {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);


    wxBoxSizer* gridSizeSizer = new wxBoxSizer(wxHORIZONTAL);
    gridSizeSizer->Add(new wxStaticText(this, wxID_ANY, "Grid Size:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    gridSizeSpinner = new wxSpinCtrl(this, 10006, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 100, 15);
    gridSizeSizer->Add(gridSizeSpinner, 1, wxEXPAND);
    mainSizer->Add(gridSizeSizer, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer* livingCellColorSizer = new wxBoxSizer(wxHORIZONTAL);
    livingCellColorSizer->Add(new wxStaticText(this, wxID_ANY, "Living Cell Color:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    livingCellColorPicker = new wxColourPickerCtrl(this, wxID_ANY);
    livingCellColorSizer->Add(livingCellColorPicker, 1, wxEXPAND);
    mainSizer->Add(livingCellColorSizer, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer* deadCellColorSizer = new wxBoxSizer(wxHORIZONTAL);
    deadCellColorSizer->Add(new wxStaticText(this, wxID_ANY, "Dead Cell Color:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    deadCellColorPicker = new wxColourPickerCtrl(this, wxID_ANY);
    deadCellColorSizer->Add(deadCellColorPicker, 1, wxEXPAND);
    mainSizer->Add(deadCellColorSizer, 0, wxEXPAND | wxALL, 10);

    wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
    if (buttonSizer)
        mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxTOP | wxBOTTOM, 10);

    SetSizerAndFit(mainSizer);
}






void SettingsDialog::OnOK(wxCommandEvent& event) {
    settings->SetLivingCellColor(livingCellColorPicker->GetColour());
    settings->SetDeadCellColor(deadCellColorPicker->GetColour());
    settings->gridSize = gridSizeSpinner->GetValue();
    settings->SaveSettingsToFile();
    EndModal(wxID_OK);
}

void SettingsDialog::OnCancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
