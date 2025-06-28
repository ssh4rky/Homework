#include <iostream>
#include "Fraction.h"

using namespace std;

int main() {
    // Main functions development
    Fraction a, b;

    cout << "Enter the first fraction:" << endl;
    a.input();

    cout << "\nEnter the second fraction:" << endl;
    b.input();

    Fraction sum = a + b;
    Fraction diff = a - b;
    Fraction prod = a * b;
    Fraction quot = a / b;

    cout << "\n==== RESULT MATRIX ====" << endl;
    cout << "Operation\tResult" << endl;
    cout << "--------\t------" << endl;

    cout << "A + B\t\t"; sum.print(); cout << endl;
    cout << "A - B\t\t"; diff.print(); cout << endl;
    cout << "A * B\t\t"; prod.print(); cout << endl;
    cout << "A / B\t\t"; quot.print(); cout << endl;

    return 0;
}
