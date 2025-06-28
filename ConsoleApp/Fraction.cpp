#include "Fraction.h"
#include <iostream>
#include <numeric>

using namespace std;

// Getting the values ready to go
Fraction::Fraction(int num, int denom) : numerator(num), denominator(denom) {
    if (denominator == 0) {
        cerr << "Error: denominator cannot be zero. Setting to 1." << endl;
        denominator = 1;
    }
    reduce();
}

// Input values
void Fraction::input() {
    cout << "Enter numerator: ";
    cin >> numerator;
    cout << "Enter denominator: ";
    cin >> denominator;

    if (denominator == 0) {
        cerr << "Error: denominator cannot be zero. Setting to 1." << endl;
        denominator = 1;
    }
    reduce();
}

// Printing the Fraction
void Fraction::print() const {
    std::cout << numerator;
    if (denominator != 1)
        std::cout << "/" << denominator;
}

// Maintaining the data source(input)
void Fraction::reduce() {
    int gcd = std::gcd(numerator, denominator);
    numerator /= gcd;
    denominator /= gcd;

    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
}

// Operators manipulations
Fraction Fraction::operator+(const Fraction& other) const {
    int num = numerator * other.denominator + other.numerator * denominator;
    int denom = denominator * other.denominator;
    return Fraction(num, denom);
}

Fraction Fraction::operator-(const Fraction& other) const {
    int num = numerator * other.denominator - other.numerator * denominator;
    int denom = denominator * other.denominator;
    return Fraction(num, denom);
}

Fraction Fraction::operator*(const Fraction& other) const {
    return Fraction(numerator * other.numerator, denominator * other.denominator);
}

Fraction Fraction::operator/(const Fraction& other) const {
    return Fraction(numerator * other.denominator, denominator * other.numerator);
}
