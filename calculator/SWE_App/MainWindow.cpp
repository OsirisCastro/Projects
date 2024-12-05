#include "MainWindow.h"
#include "ButtonFactory.h"
#include "wx/button.h"
#include <wx/tokenzr.h> 
#include <cmath>

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_BUTTON(wxID_ANY, MainWindow::OnButtonClick)
END_EVENT_TABLE()

MainWindow::MainWindow() : wxFrame(nullptr, wxID_ANY, "Calculator", wxPoint(0, 0), wxSize(500, 800)) {
    textCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxPoint(10, 10), wxSize(480, 50), wxTE_READONLY);

    ButtonFactory::InitializeNumberButtons(this);
    ButtonFactory::InitializeUnaryOperators(this);
    ButtonFactory::InitializeBinaryOperators(this);
    ButtonFactory::InitializeUtilityButtons(this);
}

void MainWindow::OnButtonClick(wxCommandEvent& event) {
    int id = event.GetId();
    wxString label;

    switch (id) {
    case ID_EQUALS:
        EvaluateExpression(textCtrl->GetValue());
        break;
    case ID_CLEAR:
        textCtrl->Clear();
        break;
    case ID_BACKSPACE: {
        wxString text = textCtrl->GetValue();
        if (!text.empty())
            textCtrl->Remove(text.length() - 1, text.length());
        break;
    }
    case ID_DECIMAL:
        textCtrl->AppendText(".");
        break;
    default:
        if (id >= ID_NUMBER_0 && id <= ID_NUMBER_9) {
            label = wxString::Format(wxT("%c"), '0' + (id - ID_NUMBER_0));
            textCtrl->AppendText(label);
        }
        else if (id >= ID_SIN && id <= ID_MODULO) {
            wxButton* button = dynamic_cast<wxButton*>(FindWindowById(id));
            textCtrl->AppendText(button->GetLabel());
        }
    }
}

void MainWindow::EvaluateExpression(const wxString& expression) {
    if (expression.IsEmpty()) {
        wxMessageBox("Expression is empty!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    CalculatorProcessor* processor = CalculatorProcessor::GetInstance();
    std::string exprStr = std::string(expression.mb_str());
    double result;
    try {
        result = processor->Calculate(exprStr);
    }
    catch (const std::exception& e) {
        wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString answer = wxString::Format(wxT("%g"), result);
    textCtrl->SetValue(answer);
}

MainWindow::~MainWindow() {}
