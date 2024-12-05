#pragma once
#include "wx/wx.h"
#include "CalculatorProcessor.h"


class MainWindow : public wxFrame {
public:
	MainWindow();
	~MainWindow();

	wxTextCtrl* textCtrl;
    wxString currentText;


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

	void OnButtonClick(wxCommandEvent& event);
	void EvaluateExpression(const wxString& expression);
    wxString IsOperator(const wxString& token);

    DECLARE_EVENT_TABLE()
};