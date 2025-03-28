#include "MainWindow.h"
#include "DrawingPanel.h"
#include <wx/numdlg.h> 
#include <ctime> 
#define TOOLBAR_PLAY 10001
#define TOOLBAR_PAUSE 10002
#define TOOLBAR_NEXT 10003
#define TOOLBAR_TRASH 10004
#define TOOLBAR_TIMER 10005
#define OPTION 10008
#define VIEW 10009
#define RANDOMIZE 10010
#define RANDOMIZE_WITH_SEED 10011
#define NEW 10012
#define OPEN 10013
#define SAVE 10014
#define SAVEAS 10015
#define EXIT 10016
#define FINITE 10017
#define TOROIDAL 10018
#define RESET 10019
#define IMPORT 10020
#define TOGGLE_10X10_GRID_EVENT 10021
#define TOGGLE_GRID_EVENT 10022
#define HUD 10023

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_SIZE(MainWindow::OnSizeChange)
EVT_MENU(TOOLBAR_PLAY, MainWindow::OnPlay)
EVT_MENU(TOOLBAR_PAUSE, MainWindow::OnPause)
EVT_MENU(TOOLBAR_NEXT, MainWindow::OnNext)
EVT_MENU(TOOLBAR_TRASH, MainWindow::OnTrash)
EVT_TIMER(TOOLBAR_TIMER, MainWindow::OnTimer)
EVT_MENU(OPTION, MainWindow::OnOpenSettings)
EVT_MENU(VIEW, MainWindow::ShowNeighborCount)
EVT_MENU(RANDOMIZE, MainWindow::OnRandomize)
EVT_MENU(RANDOMIZE_WITH_SEED, MainWindow::OnRandomizeWithSeed)
EVT_MENU(NEW, MainWindow::OnNew)
EVT_MENU(OPEN, MainWindow::OnOpen)
EVT_MENU(SAVE, MainWindow::OnSave)
EVT_MENU(SAVEAS, MainWindow::OnSaveAs)
EVT_MENU(EXIT, MainWindow::OnExit)
EVT_MENU(FINITE, MainWindow::OnFinite)
EVT_MENU(TOROIDAL, MainWindow::OnToroidal)
EVT_MENU(RESET, MainWindow::OnResetSettings)
EVT_MENU(IMPORT, MainWindow::OnImport)
EVT_MENU(TOGGLE_10X10_GRID_EVENT, MainWindow::OnToggle10x10Grid)
EVT_MENU(TOGGLE_GRID_EVENT, MainWindow::OnToggleGrid)
EVT_MENU(HUD, MainWindow::OnToggleHUD)

wxEND_EVENT_TABLE()

MainWindow::MainWindow() : wxFrame(nullptr, wxID_ANY, "Game of Life", wxPoint(0, 0), wxSize(200, 200)) {
	settings.LoadSettingsFromFile();

	statusBar = CreateStatusBar(2);
	toolBar = CreateToolBar();
	wxBitmap playBitmap(play_xpm);
	wxBitmap nextBitmap(next_xpm);
	wxBitmap pauseBitmap(pause_xpm);
	wxBitmap trashBitmap(trash_xpm);

	toolBar->AddTool(TOOLBAR_PLAY, "Play", playBitmap);
	toolBar->AddTool(TOOLBAR_PAUSE, "Pause", pauseBitmap);
	toolBar->AddTool(TOOLBAR_NEXT, "Next", nextBitmap);
	toolBar->AddTool(TOOLBAR_TRASH, "Trash", trashBitmap);
	drawingPanel = new DrawingPanel(this, gameBoard, &settings);
	UpdateStatusBar();
	InitializedGrid();

	timer = new wxTimer(this, TOOLBAR_TIMER);
	timer->Start(100);

	menuBar = new wxMenuBar();

	fileMenu = new wxMenu();
	fileMenu->Append(NEW, "&New");
	fileMenu->Append(OPEN, "&Open");
	fileMenu->Append(SAVE, "&Save");
	fileMenu->Append(SAVEAS, "Save &As");
	fileMenu->Append(EXIT, "&Exit");
	menuBar->Append(fileMenu, "&File");

	optionsMenu = new wxMenu();
	optionsMenu->Append(OPTION, "Settings");
	optionsMenu->Append(RANDOMIZE, "Randomize");
	optionsMenu->Append(RANDOMIZE_WITH_SEED, "Randomize with Seed");
	menuBar->Append(optionsMenu, "&Options");

	viewMenu = new wxMenu();
	showNeighborCount = new wxMenuItem(viewMenu, VIEW, "Show Neighbor Count", " ", wxITEM_CHECK);
	showNeighborCount->SetCheckable(true);
	showNeighborCount->Check(false);
	viewMenu->Append(showNeighborCount);
	menuBar->Append(viewMenu, "View");

	wxMenuItem* finiteOption = new wxMenuItem(viewMenu, FINITE, "Finite", "Use Finite Universe", wxITEM_CHECK);
	finiteOption->SetCheckable(true);
	finiteOption->Enable(!settings.toroidal);
	viewMenu->Append(finiteOption);

	wxMenuItem* toroidalOption = new wxMenuItem(viewMenu, TOROIDAL, "Toroidal", "Use Toroidal Universe", wxITEM_CHECK);
	toroidalOption->SetCheckable(true);
	toroidalOption->Check(settings.toroidal);
	viewMenu->Append(toroidalOption);

	optionsMenu->Append(RESET, "&Reset Settings\tCtrl+R", "Reset all settings to default");
	fileMenu->Append(IMPORT, "&Import");

	wxMenuItem* toogleGridItem = new wxMenuItem(viewMenu, TOGGLE_GRID_EVENT, "Show Grid", "", wxITEM_CHECK);
	wxMenuItem* toogle10x10GridItem = new wxMenuItem(viewMenu, TOGGLE_10X10_GRID_EVENT, "Show 10x10 Grid", "", wxITEM_CHECK);
	toogleGridItem->SetCheckable(true);
	toogle10x10GridItem->SetCheckable(true);
	viewMenu->Append(toogleGridItem);
	viewMenu->Append(toogle10x10GridItem);

	wxMenuItem* toogleHUD = new wxMenuItem(viewMenu, HUD, "Toogle HUD", "", wxITEM_CHECK);
	viewMenu->Append(toogleHUD);
	settings.showGrid = true;

	SetMenuBar(menuBar);

	toolBar->Realize();
	this->Layout();
}

MainWindow::~MainWindow() {
	delete drawingPanel;
	delete timer;

}

void MainWindow::InitializedGrid() {

	gameBoard.resize(settings.gridSize);
	for (auto& gameTile : gameBoard) {
		gameTile.resize(settings.gridSize);
	}

	if (drawingPanel) {

		drawingPanel->SetGridSize(settings.gridSize);
		drawingPanel->Refresh();
		drawingPanel->Update();
	}
}

void MainWindow::OnSizeChange(wxSizeEvent& event) {

	wxSize size = GetClientSize();
	drawingPanel->SetSize(size);
	event.Skip();

}

void MainWindow::UpdateStatusBar() {

	if (statusBar)
	{
		wxString StatusText = wxString::Format("Generation: %d, Living Cells: %d", drawingPanel->generation, drawingPanel->liveCells);
		statusBar->SetStatusText(StatusText);
	}
}

void MainWindow::OnPlay(wxCommandEvent& playBut) {
	timer->Start(1000);

}
void MainWindow::OnPause(wxCommandEvent& pauseBut) {
	timer->Stop();

}
void MainWindow::OnNext(wxCommandEvent& nextBut) {

	MainWindow::NextGeneration();

}
void MainWindow::OnTrash(wxCommandEvent& trashBut) {

	for (int i = 0; i < settings.gridSize; i++) {
		for (int j = 0; j < settings.gridSize; j++) {
			gameBoard[i][j] = false;
		}
	}

	drawingPanel->generation = 0;
	drawingPanel->liveCells = 0;
	UpdateStatusBar();
	Refresh();
}

void MainWindow::OnTimer(wxTimerEvent& event) {
	NextGeneration();
}

void MainWindow::OnOpenSettings(wxCommandEvent& event) {

	SettingsDialog settingsDialog(this, &settings);

	if (settingsDialog.ShowModal() == wxID_OK) {
		InitializedGrid();
		drawingPanel->Refresh();
		event.Skip();

	}

}

void MainWindow::ShowNeighborCount(wxCommandEvent& event) {

	if (settings.showNeighborCount)
	{
		settings.showNeighborCount = false;
	}
	else
	{
		settings.showNeighborCount = true;
	}

	Refresh();
	event.Skip();
}

void MainWindow::OnFinite(wxCommandEvent& event) {
	settings.SetUniverseType(UniverseType::Finite);
	drawingPanel->Refresh();
}

void MainWindow::OnToroidal(wxCommandEvent& event) {
	settings.SetUniverseType(UniverseType::Toroidal);
	drawingPanel->Refresh();
}

void MainWindow::OnRandomize(wxCommandEvent& event) {
	long seed = time(NULL);
	RandomizeGrid(seed);
}

void MainWindow::OnRandomizeWithSeed(wxCommandEvent& event) {

	long seed = wxGetNumberFromUser("Enter Seed", "Seed:", "Randomize with Seed", 15, 0, LONG_MAX);
	if (seed != -1) {
		RandomizeGrid(seed);
	}
}

void MainWindow::RandomizeGrid(int seed) {
	srand(seed);
	for (int i = 0; i < settings.gridSize; i++) {
		for (int j = 0; j < settings.gridSize; j++) {
			gameBoard[i][j] = rand() % 2 == 0;
		}
	}

	drawingPanel->generation = 0;
	drawingPanel->liveCells = 0;
	UpdateStatusBar();
	Refresh();
}

void MainWindow::OnNew(wxCommandEvent& event) {
	gameBoard.clear();
	InitializedGrid();
	currentFilename.Clear();
}

void MainWindow::OnOpen(wxCommandEvent& event) {
	wxFileDialog openFileDialog(this, _("Open Game Board File"), "", "", "Game Board Files (*.cells)|*.cells", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	wxString filename = openFileDialog.GetPath();
	LoadGame(filename);
	currentFilename = filename;
}

void MainWindow::OnSave(wxCommandEvent& event) {
	if (currentFilename.empty()) {
		OnSaveAs(event);
	}
	else {
		SaveGame(currentFilename);
	}
}

void MainWindow::OnSaveAs(wxCommandEvent& event) {
	wxFileDialog saveFileDialog(this, _("Save Game Board File"), "", "", "Game Board Files (*.cells)|*.cells", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;

	wxString filename = saveFileDialog.GetPath();
	SaveGame(filename);
	currentFilename = filename;
}

void MainWindow::OnExit(wxCommandEvent& event) {
	Close();
}

void MainWindow::SaveGame(const wxString& filename) {
	std::ofstream file(filename.ToStdString());
	if (!file.is_open()) {
		wxMessageBox("Failed to save file.", "Error", wxICON_ERROR | wxOK);
		return;
	}

	for (const auto& row : gameBoard) {
		for (bool cell : row) {
			file << (cell ? '*' : '.');
		}
		file << '\n';
	}

	file.close();
}

void MainWindow::LoadGame(const wxString& filename) {
	std::ifstream file(filename.ToStdString());
	if (!file.is_open()) {
		wxMessageBox("Failed to open file.", "Error", wxICON_ERROR | wxOK);
		return;
	}

	gameBoard.clear();
	std::string line;
	while (std::getline(file, line)) {
		std::vector<bool> row;
		for (char c : line) {
			row.push_back(c == '*');
		}
		gameBoard.push_back(row);
	}

	file.close();

	InitializedGrid();
}

void MainWindow::OnImport(wxCommandEvent& event) {
	wxFileDialog openFileDialog(this, _("Import Game Board File"), "", "", "Game Board Files (*.cells)|*.cells", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	wxString filename = openFileDialog.GetPath();
	ImportGame(filename);
	currentFilename = filename;
}

void MainWindow::OnToggle10x10Grid(wxCommandEvent& event) {
	settings.show10x10Grid = event.IsChecked();
	drawingPanel->Refresh();
}

void MainWindow::OnToggleGrid(wxCommandEvent& event) {
	settings.showGrid = event.IsChecked();
	drawingPanel->Refresh();
}

void MainWindow::OnToggleHUD(wxCommandEvent& event) {
	settings.SetShowHUD(event.IsChecked());
	drawingPanel->Refresh();
}

void MainWindow::ImportGame(const wxString& filename) {
	std::ifstream file(filename.ToStdString());
	if (!file.is_open()) {
		wxMessageBox("Failed to open file.", "Error", wxICON_ERROR | wxOK);
		return;
	}

	std::vector<std::vector<bool>> importedPattern;
	std::string line;
	while (std::getline(file, line)) {
		std::vector<bool> row;
		for (char c : line) {
			row.push_back(c == '*');
		}
		importedPattern.push_back(row);
	}

	file.close();

	Refresh();
}

int MainWindow::CountLivingNeighbors(int a, int b) {

	int livingcount = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {

			if (i == 0 && j == 0) { continue; }
			int x = a + i;
			int y = b + j;

			if (settings.universeType == UniverseType::Toroidal) {

				x = (x + settings.gridSize) % settings.gridSize;
				y = (y + settings.gridSize) % settings.gridSize;
			}

			if (x >= 0 && x < settings.gridSize && y >= 0 && y < settings.gridSize) {
				if (gameBoard[x][y]) { livingcount++; }

			}
		}
	}
	Refresh();
	return livingcount;
}

void MainWindow::OnResetSettings(wxCommandEvent& event) {

	settings.ResetToDefault();
	gameBoard.resize(settings.gridSize);
	for (int i = 0; i < settings.gridSize; ++i)
	{
		gameBoard[i].resize(settings.gridSize);
	}

	Refresh();
}

void MainWindow::NextGeneration() {
	std::vector<std::vector<bool>> sandbox;
	sandbox.resize(settings.gridSize);

	for (auto& cell : sandbox) {
		cell.resize(settings.gridSize);
	}

	for (int i = 0; i < settings.gridSize; i++) {
		for (int j = 0; j < settings.gridSize; j++) {
			int liveCell = CountLivingNeighbors(i, j);

			if (gameBoard[i][j] && (liveCell < 2 || liveCell > 3)) {
				sandbox[i][j] = false;
			}
			if (gameBoard[i][j] && (liveCell == 2 || liveCell == 3)) {
				sandbox[i][j] = true;
			}
			if (gameBoard[i][j] == false && liveCell == 3) {
				sandbox[i][j] = true;
			}
		}
	}

	swap(gameBoard, sandbox);
	for (int i = 0; i < settings.gridSize; i++) {
		for (int j = 0; j < settings.gridSize; j++) {
			if (gameBoard[i][j]) {
				drawingPanel->liveCells++;
			}
		}
	}

	drawingPanel->generation++;
	UpdateStatusBar();
	Refresh();


}
