#pragma once
#include <iostream>

// Class initialization

class Fraction {
private:
    int numerator;
    int denominator;

    void reduce(); // the function to maintain data

public:
    Fraction(int num = 0, int denom = 1);

    void input();        // Input from user
    void print() const;  // Print the fraction

    // For operators we need the classtype-based expressions(initialize values with each operator)
    Fraction operator+(const Fraction& other) const;
    Fraction operator-(const Fraction& other) const;
    Fraction operator*(const Fraction& other) const;
    Fraction operator/(const Fraction& other) const;
};
