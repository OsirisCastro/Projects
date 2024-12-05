#include "../SWE_APP/CalculatorProcessor.cpp"
#include "../SWE_App/ButtonFactory.h"
#include "../SWE_App/MainWindow.h"
#include "CppUnitTest.h"
#include <wx/tokenzr.h>
#include <queue>
#include <stack>
#include <sstream>
#include <iomanip> 


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CalculatorProcessorTest
{
    TEST_CLASS(CalculatorProcessorTest)
    {
    public:

        CalculatorProcessor* Calcfun = CalculatorProcessor::GetInstance();
        wxWindow* parent = new wxFrame(nullptr, wxID_ANY, "Test Parent Window");

        std::string DoubleToString(double value, int precision = 6) {
            std::ostringstream out;
            out << std::fixed << std::setprecision(precision) << value;
            return out.str();
        }

        TEST_METHOD(Add)
        {
            double result = Calcfun->Calculate("2+2");
            Assert::AreEqual(std::string("4.000000"), DoubleToString(result));
        }

        TEST_METHOD(Subtract)
        {
            double result = Calcfun->Calculate("9-2");
            Assert::AreEqual(std::string("7.000000"), DoubleToString(result));
        }

        TEST_METHOD(Divide)
        {
            double result = Calcfun->Calculate("8/2");
            Assert::AreEqual(std::string("4.000000"), DoubleToString(result));
        }

        TEST_METHOD(Multiply)
        {
            double result = Calcfun->Calculate("10*2");
            Assert::AreEqual(std::string("20.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex01)
        {
            double result = Calcfun->Calculate("9-2+3");
            Assert::AreEqual(std::string("10.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex02)
        {
            double result = Calcfun->Calculate("9*2+3");
            Assert::AreEqual(std::string("21.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex03)
        {
            double result = Calcfun->Calculate("8/2*3+4");
            Assert::AreEqual(std::string("16.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex04)
        {
            double result = Calcfun->Calculate("9+4+6+9");
            Assert::AreEqual(std::string("28.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex05)
        {
            double result = Calcfun->Calculate("9-10-3-6");
            Assert::AreEqual(std::string("-10.000000"), DoubleToString(result));
        }

        TEST_METHOD(Complex06)
        {
            double result = Calcfun->Calculate("9*3/2+100");
            Assert::AreEqual(std::string("113.500000"), DoubleToString(result));
        }
    };
}
