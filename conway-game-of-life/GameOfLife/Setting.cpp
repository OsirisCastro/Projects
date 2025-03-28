#include "Setting.h"
#include "DrawingPanel.h"

wxColor Setting::GetLivingCellColor() const
{
    return wxColor(livingCellColorRed, livingCellColorGreen, livingCellColorBlue, livingCellColorAlpha);
}

wxColor Setting::GetDeadCellColor() const
{
    return wxColor(deadCellColorRed, deadCellColorGreen, deadCellColorBlue, deadCellColorAlpha);
}

void Setting::SetLivingCellColor(const wxColor& color)
{
    livingCellColorRed = color.Red();
    livingCellColorGreen = color.Green();
    livingCellColorBlue = color.Blue();
    livingCellColorAlpha = color.Alpha();
}

void Setting::SetDeadCellColor(const wxColor& color)
{
    deadCellColorRed = color.Red();
    deadCellColorGreen = color.Green();
    deadCellColorBlue = color.Blue();
    deadCellColorAlpha = color.Alpha();
}

bool Setting::getShowHUD()
{
    return showHUD;
}

void Setting::SetShowHUD(bool ShowHUD) {
    showHUD = ShowHUD;
}

void Setting::ResetToDefault() {

    livingCellColorRed = 128;
    livingCellColorGreen = 128;
    livingCellColorBlue = 128;
    livingCellColorAlpha = 255;

    deadCellColorRed = 255;
    deadCellColorGreen = 255;
    deadCellColorBlue = 255;
    deadCellColorAlpha = 255;

    gridSize = 15;
    interval = 50;
    toroidal = false;
    showNeighborCount = false;

    SaveSettingsToFile();

}
