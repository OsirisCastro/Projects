#pragma once

#include <wx/wx.h>
#include "Setting.h"

class DrawingPanel : public wxPanel {
public:
    DrawingPanel(wxFrame* parent, std::vector<std::vector<bool>>& board, Setting* settings);
    ~DrawingPanel();
    void SetSize(const wxSize& size);
    void SetGridSize(int size);
    int generation = 0;
    int liveCells = 0;

private:
    std::vector<std::vector<bool>>& gameBoard;
    std::vector<std::vector<int>> neighborCount;
    Setting* settings;
    int gridSize = 15;
    int cellSize = 20;



    void OnPaint(wxPaintEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    int NeighborCount(int& x, int& y);
    void UpdateNeighborCount();
};
