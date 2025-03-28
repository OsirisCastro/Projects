#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "play.xpm"
#include "pause.xpm"
#include "next.xpm"
#include "trash.xpm"


#include <wx/wx.h>
#include "DrawingPanel.h"
#include <vector>
#include "Setting.h"
#include "SettingsDialog.h"

class MainWindow : public wxFrame {
public:
    MainWindow();
    ~MainWindow();
    void OnSizeChange(wxSizeEvent& event);
    void OnPlay(wxCommandEvent& playBut);
    void OnPause(wxCommandEvent& pauseBut);
    void OnNext(wxCommandEvent& nextBut);
    void OnTrash(wxCommandEvent& trashBut);
    void OnTimer(wxTimerEvent& event);
    int CountLivingNeighbors(int a, int b);
    void NextGeneration();
    void OnOpenSettings(wxCommandEvent& event);
    void ShowNeighborCount(wxCommandEvent& event);
    void OnRandomize(wxCommandEvent& event);
    void OnRandomizeWithSeed(wxCommandEvent& event);
    void RandomizeGrid(int seed);
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnFinite(wxCommandEvent& event);
    void OnToroidal(wxCommandEvent& event);
    void OnResetSettings(wxCommandEvent& event);
    void ImportGame(const wxString& filename);
    void OnImport(wxCommandEvent& event);
    void OnToggle10x10Grid(wxCommandEvent& event);
    void OnToggleGrid(wxCommandEvent& event);
    void OnToggleHUD(wxCommandEvent& event);
    DrawingPanel* drawingPanel;

private:

    std::vector<std::vector<bool>> gameBoard;
    Setting settings;

    wxStatusBar* statusBar = nullptr;
    wxToolBar* toolBar = nullptr;
    wxTimer* timer = nullptr;
    wxMenuBar* menuBar = nullptr;
    wxMenu* optionsMenu = nullptr;
    wxMenu* viewMenu = nullptr;
    wxMenuItem* showNeighborCount = nullptr;
    wxString currentFilename;
    wxMenu* fileMenu = nullptr;

    void InitializedGrid();
    void UpdateStatusBar();
    void SaveGame(const wxString& filename);
    void LoadGame(const wxString& filename);

    bool showGrid;
    bool show10x10Grid;

    wxDECLARE_EVENT_TABLE();
};


#endif 
