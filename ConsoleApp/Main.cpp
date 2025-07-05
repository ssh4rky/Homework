#include <iostream>

using namespace std;

class Date {
protected:
	// Date inizialization
	int day;
	int month;
	int year;

	int getDaysInMonth(int m, int y) {
		if (m == 2) {
			// Leap year
			if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
				return 29;
			else
				return 28;
		}
		else if (m == 4 || m == 6 || m == 9 || m == 11)
			return 30;
		else
			return 31;
	}
public:
	Date() {
		static bool seeded = false;
		if (!seeded) {
			// function to delay the srand signation
			srand(static_cast<unsigned int>(time(0)));
			seeded = true;
		}

		// Generation
		month = rand() % 12 + 1; // 12 months
		year = rand() % (2025 - 1900 + 1) + 1900; // from 1900 to 2025

		int maxDay = getDaysInMonth(month, year);
		day = rand() % maxDay + 1;     // 1–28/29/30/31
	}

	// Getter

	void showDate() {
		cout << "Generated date: " << day << "." << month << "." << year << endl;
	}

	// Copy Constructor(Parent)
	Date(const Date& other) {
		this->day = other.day;
		this->month = other.month;
		this->year = other.year;
	}
};

class Person : public Date{
private:
	// ID
	char* identification;

	char* generateRandomID() {
		int number = rand() % 10000;
		char* id = new char[5];
		sprintf_s(id, 5, "%04d", number);
		return id;
	}
	// Name
	char* name;
	char* surname;
	char* transliteration;
	// Birthdate
	// Parental class Date
	// Count for amount of objects
	static int count;
public:
	Person() : Person(generateRandomID(), "NoName", "NoSurname", "N/A") {};

	// Main constructor
	Person(const char* identification, const char* name, const char* surname, const char* transliteration) {
		this->identification = new char[strlen(identification) + 1];
		this->name = new char[strlen(name) + 1];
		this->surname = new char[strlen(surname) + 1];
		this->transliteration = new char[strlen(transliteration) + 1];

		strcpy_s(this->identification, strlen(identification) + 1, identification);
		strcpy_s(this->name, strlen(name) + 1, name);
		strcpy_s(this->surname, strlen(surname) + 1, surname);
		strcpy_s(this->transliteration, strlen(transliteration) + 1, transliteration);

		count++;
	}

	// Copy constructor(Child)
	Person(const Person& other) : Date(other) {
		identification = generateRandomID();
		name = new char[strlen(other.name) + 1];
		surname = new char[strlen(other.surname) + 1];
		transliteration = new char[strlen(other.transliteration) + 1];

		strcpy_s(name, strlen(other.name) + 1, other.name);
		strcpy_s(surname, strlen(other.surname) + 1, other.surname);
		strcpy_s(transliteration, strlen(other.transliteration) + 1, other.transliteration);

		count++;
	}

	// Destructor
	~Person() {
		delete[] identification;
		delete[] name;
		delete[] surname;
		delete[] transliteration;
		count--;
	}

	// Setters
	Person& SetName(const char* newName = "Ivan") {
		delete[] name;
		name = new char[strlen(newName) + 1];
		strcpy_s(name, strlen(newName) + 1, newName);
		return *this;
	}

	Person& SetSurname(const char* newSurname = "Petrov") {
		delete[] surname;
		surname = new char[strlen(newSurname) + 1];
		strcpy_s(surname, strlen(newSurname) + 1, newSurname);
		return *this;
	}

	Person& SetTransliteration(const char* newTrans = "Ivan Petrov") {
		delete[] transliteration;
		transliteration = new char[strlen(newTrans) + 1];
		strcpy_s(transliteration, strlen(newTrans) + 1, newTrans);
		return *this;
	}

	// Getter
	void GetInfo() {
		cout << "ID: " << identification << '\n';
		cout << "Name: " << name << '\n';
		cout << "Surname: " << surname << '\n';
		cout << "Transliteration: " << transliteration << '\n';
		showDate();
		cout << endl;
	}

	// Static

	static int GetCount() {
		return count;
	}
};

int Person::count = 0;

int main()
{
	Person p1;
	p1.SetName("Igor").SetSurname("Shevchenko").SetTransliteration("Oleksandrovych");

	Person p2(p1); // copy
	Person p3(p2); // copy of a copy
	Person p4(p3); // copy of a copy of a copy

	cout << "Original person: ";
	p1.GetInfo();

	cout << "\nCopied person: ";
	p2.GetInfo();

	cout << "\nCopied to copy person: ";
	p3.GetInfo();

	cout << "\nCopied to copy which is copied person: ";
	p4.GetInfo();

	cout << "\nTotal persons created: " << Person::GetCount() << endl;
}

