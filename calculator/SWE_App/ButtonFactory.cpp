#include "ButtonFactory.h"

wxButton* ButtonFactory::CreateButton(wxWindow* parent, int id, const wxString& label, const wxPoint& pos, const wxSize& size) {
    return new wxButton(parent, id, label, pos, size);
}

void ButtonFactory::InitializeNumberButtons(wxWindow* parent) {
    for (int i = 0; i <= 9; ++i) {
        wxString label = wxString::Format(wxT("%d"), i);
        int row = i / 3;
        int col = i % 3;
        CreateButton(parent, ID_NUMBER_0 + i, label, wxPoint(10 + col * 100, 80 + row * 80), wxSize(80, 80));
    }
}

void ButtonFactory::InitializeUnaryOperators(wxWindow* parent) {
    wxString unaryOperators[] = { wxT("sin"), wxT("cos"), wxT("tan") };
    for (int i = 0; i < 3; ++i) {
        int row = i / 3;
        int col = i % 3;
        CreateButton(parent, ID_SIN + i, unaryOperators[i], wxPoint(10 + col * 100, 400 + row * 80), wxSize(80, 80));
    }
}

void ButtonFactory::InitializeBinaryOperators(wxWindow* parent) {
    wxString binaryOperators[] = { "+", "-", "*", "/", "%" };
    for (int i = 0; i < 5; ++i) {
        CreateButton(parent, ID_ADD + i, binaryOperators[i], wxPoint(310, 80 + i * 80), wxSize(80, 80));
    }
}

void ButtonFactory::InitializeUtilityButtons(wxWindow* parent) {
    CreateButton(parent, ID_EQUALS, "=", wxPoint(110, 320), wxSize(80, 80));
    CreateButton(parent, ID_CLEAR, "Clear", wxPoint(10, 480), wxSize(180, 80));
    CreateButton(parent, ID_BACKSPACE, "Backspace", wxPoint(210, 480), wxSize(180, 80));
    CreateButton(parent, ID_DECIMAL, ".", wxPoint(210, 320), wxSize(80, 80));
}
