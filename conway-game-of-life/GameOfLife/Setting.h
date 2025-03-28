#pragma once
#include <wx/wx.h>
#include <fstream>

enum UniverseType {
    Finite,
    Toroidal
};

struct Setting {

    unsigned int livingCellColorRed = 128;
    unsigned int livingCellColorGreen = 128;
    unsigned int livingCellColorBlue = 128;
    unsigned int livingCellColorAlpha = 255;

    unsigned int deadCellColorRed = 255;
    unsigned int deadCellColorGreen = 255;
    unsigned int deadCellColorBlue = 255;
    unsigned int deadCellColorAlpha = 255;

    int gridSize = 15;
    int interval = 50;

    wxColor GetLivingCellColor() const;
    wxColor GetDeadCellColor() const;
    void SetLivingCellColor(const wxColor& color);
    void SetDeadCellColor(const wxColor& color);
    bool showNeighborCount = false;
    bool toroidal = false;
    bool finite = false;
    bool showGrid = true;
    bool show10x10Grid = false;
    bool showHUD = false;
    bool getShowHUD();
    void SetShowHUD(bool ShowHUD);
    UniverseType universeType;
    UniverseType GetUniverseType() const {
        return universeType;
    }
    void SetUniverseType(UniverseType type) {
        universeType = type;
    }

    void ResetToDefault();

    void SaveSettingsToFile() {
        std::ofstream file("settings.bin", std::ios::out | std::ios::binary);
        file.write((char*)this, sizeof(Setting));
        file.close();
    }

    void LoadSettingsFromFile() {
        std::ifstream file("settings.bin", std::ios::binary | std::ios::in);
        file.read((char*)this, sizeof(Setting));
        file.close();
    }
};
