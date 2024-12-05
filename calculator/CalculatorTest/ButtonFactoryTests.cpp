#include "../SWE_APP/CalculatorProcessor.h"
#include "../SWE_App/ButtonFactory.cpp"
#include "../SWE_App/MainWindow.h"
#include "CppUnitTest.h"
#include <wx/tokenzr.h>
#include <queue>
#include <stack>
#include <sstream>
#include <iomanip> 


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ButtonProcessorTest
{
    TEST_CLASS(ButtonProcessorTest)
    {
    public:
        wxWindow* Parent = new wxFrame(nullptr, wxID_ANY, "Test Parent Window");

        wxPoint defaultPos = wxPoint(0, 0);
        wxSize defaultSize = wxSize(50, 50);

        TEST_METHOD(ButtonTestMethod1)
        {
            wxButton* AddButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_ADD, "+", defaultPos, defaultSize);
            Assert::IsNotNull(AddButton, L"Addition button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod2)
        {
            wxButton* SubtractButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_SUBTRACT, "-", defaultPos, defaultSize);
            Assert::IsNotNull(SubtractButton, L"Subtraction button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod3)
        {
            wxButton* MultiplyButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_MULTIPLY, "*", defaultPos, defaultSize);
            Assert::IsNotNull(MultiplyButton, L"Multiply button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod4)
        {
            wxButton* DivideButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_DIVIDE, "/", defaultPos, defaultSize);
            Assert::IsNotNull(DivideButton, L"Divide button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod5)
        {
            wxButton* EqualButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_EQUALS, "=", defaultPos, defaultSize);
            Assert::IsNotNull(EqualButton, L"Equal button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod6)
        {
            wxButton* ClearButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_CLEAR, "C", defaultPos, defaultSize);
            Assert::IsNotNull(ClearButton, L"Clear button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod7)
        {
            wxButton* BackSpaceButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_BACKSPACE, "BS", defaultPos, defaultSize);
            Assert::IsNotNull(BackSpaceButton, L"BackSpace button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod8)
        {
            wxButton* Num0Button = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_NUMBER_0, "0", defaultPos, defaultSize);
            Assert::IsNotNull(Num0Button, L"Number 0 button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod9)
        {
            wxButton* Num1Button = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_NUMBER_1, "1", defaultPos, defaultSize);
            Assert::IsNotNull(Num1Button, L"Number 1 button Creation Failed.");
        }

        TEST_METHOD(ButtonTestMethod10)
        {
            wxButton* SinButton = ButtonFactory::CreateButton(Parent, ButtonFactory::ID_SIN, "sin", defaultPos, defaultSize);
            Assert::IsNotNull(SinButton, L"Sin button Creation Failed.");
        }
    };
}
