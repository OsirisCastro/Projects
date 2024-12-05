#pragma once
#include "wx/wx.h"

class ButtonFactory {
public:
    static void InitializeNumberButtons(wxWindow* parent);
    static void InitializeUnaryOperators(wxWindow* parent);
    static void InitializeBinaryOperators(wxWindow* parent);
    static void InitializeUtilityButtons(wxWindow* parent);


    enum {
        ID_NUMBER_0 = wxID_HIGHEST + 1,
        ID_NUMBER_1,
        ID_NUMBER_2,
        ID_NUMBER_3,
        ID_NUMBER_4,
        ID_NUMBER_5,
        ID_NUMBER_6,
        ID_NUMBER_7,
        ID_NUMBER_8,
        ID_NUMBER_9,
        ID_SIN,
        ID_COS,
        ID_TAN,
        ID_ADD,
        ID_SUBTRACT,
        ID_MULTIPLY,
        ID_DIVIDE,
        ID_MODULO,
        ID_EQUALS,
        ID_CLEAR,
        ID_BACKSPACE,
        ID_DECIMAL,
        
    };

    static wxButton* CreateButton(wxWindow* parent, int id, const wxString& label, const wxPoint& pos, const wxSize& size);
};
