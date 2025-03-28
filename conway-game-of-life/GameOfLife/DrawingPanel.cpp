#include "DrawingPanel.h"
#include "MainWindow.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

DrawingPanel::DrawingPanel(wxFrame* parent, std::vector<std::vector<bool>>& board, Setting* settings) : wxPanel(parent), gameBoard(board), settings(settings) {
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &DrawingPanel::OnPaint, this);
    Bind(wxEVT_LEFT_UP, &DrawingPanel::OnMouseUp, this);
}

DrawingPanel::~DrawingPanel() {}

void DrawingPanel::OnPaint(wxPaintEvent& event) {

    wxAutoBufferedPaintDC Paint(this);
    Paint.Clear();

    wxGraphicsContext* context = wxGraphicsContext::Create(Paint);

    if (!context) return;

    if (settings->showGrid)
    {
        context->SetPen(*wxBLACK);
    }

    if (settings->showGrid == false)
    {
        context->SetPen(*wxWHITE);
    }


    int width, height;

    GetClientSize(&width, &height);
    int CellWidth = width / gridSize;
    int CellHeight = height / gridSize;



    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            int X = j * CellWidth;
            int Y = i * CellHeight;

            if (gameBoard[i][j]) {
                context->SetBrush(wxColor(settings->GetLivingCellColor()));
            }

            else {
                context->SetBrush(wxColor(settings->GetDeadCellColor()));
            }

            context->DrawRectangle(X, Y, CellWidth, CellHeight);
        }
    }


    if (settings->show10x10Grid == true) {
        int solidLines = settings->gridSize / 10;
        wxPen thickerpen(*wxBLACK, 3);
        Paint.SetPen(thickerpen);
        for (int i = 0; i <= solidLines; i++) {

            wxPoint hStart(0, CellHeight * i * 10);
            wxPoint hEnd(this->GetSize().x, CellHeight * i * 10);
            Paint.DrawLine(hStart, hEnd);
        }
        for (int i = 0; i <= solidLines; i++) {
            wxPoint vStart(CellWidth * i * 10, 0);
            wxPoint vEnd(CellWidth * i * 10, this->GetSize().y);
            Paint.DrawLine(vStart, vEnd);
        }
    }

    if (settings->showNeighborCount) {
        Paint.SetFont(wxFontInfo(16));
        Paint.SetTextForeground(*wxRED);
        for (int i = 0; i < settings->gridSize; i++) {
            UpdateNeighborCount();
            for (int j = 0; j < settings->gridSize; j++) {
                wxString NeighborCountText = wxString::Format("%d", neighborCount[i][j]);
                int x, y;
                Paint.GetTextExtent(NeighborCountText, &x, &y);
                {
                    wxCoord xPosition = (i * CellWidth) + (CellWidth / 2) - (x / 2);
                    wxCoord yPosition = (j * CellHeight) + (CellHeight / 2) - (y / 2);
                    Paint.DrawText(NeighborCountText, xPosition, yPosition);
                }

            }
        }
    }

    if (settings->showHUD == true) {
        std::string type;
        if (settings->universeType == UniverseType::Toroidal) {
            type = "Torodial";
        }
        else {
            type = "Finite";
        }
        context->SetFont(wxFontInfo(16), *wxRED);
        wxString hudText;
        hudText << "Generation: " << generation << "\n"
            << "Living Cells: " << liveCells << "\n"
            << "Boundary type: " << type << "\n"
            << "Universe Size: " << settings->gridSize / 2 << "x" << settings->gridSize / 2;

        double textWidth, textHeight;
        context->GetTextExtent(hudText, &textWidth, &textHeight);

        double x = 10;
        double y = GetSize().GetHeight() - textHeight - 10;

        context->DrawText(hudText, x, y);
    }

    delete context;
}


void DrawingPanel::SetSize(const wxSize& size) {
    wxPanel::SetSize(size);
    Refresh();
}

void DrawingPanel::SetGridSize(int size) {
    gridSize = size;
    Refresh();
}

void DrawingPanel::OnMouseUp(wxMouseEvent& event) {

    int mouseX = event.GetX();
    int mouseY = event.GetY();

    int width, height;

    GetClientSize(&width, &height);
    int cellWidht = width / gridSize;
    int cellHeight = height / gridSize;

    int row = mouseY / cellHeight;
    int column = mouseX / cellWidht;

    if (row >= 0 && row < gridSize && column >= 0 && column < gridSize) {
        gameBoard[row][column] = !gameBoard[row][column];

        Refresh();
    }
}

int DrawingPanel::NeighborCount(int& x, int& y) {

    int livingcount = 0;
    for (int i = -1; i < 2; i++) {

        for (int j = -1; j < 2; j++) {

            if (x + i < 0 || y + j < 0) { continue; }
            if (x + i >= settings->gridSize || y + j >= settings->gridSize) { continue; }
            if (i == 0 && j == 0) { continue; }
            if (gameBoard[x + i][y + j]) {
                livingcount++;
            }
        }
    }

    return livingcount;
}

void DrawingPanel::UpdateNeighborCount() {

    neighborCount.clear();
    neighborCount.resize(settings->gridSize);
    for (auto& i : neighborCount)
        i.resize(settings->gridSize);
    for (int i = 0; i < settings->gridSize; i++) {
        for (int j = 0; j < settings->gridSize; j++) {

            neighborCount[i][j] = NeighborCount(i, j);
        }
    }
}
