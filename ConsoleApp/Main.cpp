#include <iostream>

using namespace std;

struct Date {
	int day;
	int month;
	int year;

	int getDaysInMonth(int m, int y) {
		if (m == 2) {
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

	Date() {
		static bool seeded = false;
		if (!seeded) {
			srand(static_cast<unsigned int>(time(0)));
			seeded = true;
		}
		month = rand() % 12 + 1;
		year = rand() % (2025 - 1900 + 1) + 1900;
		int maxDay = getDaysInMonth(month, year);
		day = rand() % maxDay + 1;
	}

	Date(const Date& other) {
		day = other.day;
		month = other.month;
		year = other.year;
	}

	void showDate() {
		cout << "Generated date: " << day << "." << month << "." << year << endl;
	}
};

class Person : public Date {
private:
	char* identification;

	char* generateRandomID() {
		int number = rand() % 10000;
		char* id = new char[5];
		sprintf_s(id, 5, "%04d", number);
		return id;
	}

	char* name;
	char* surname;
	char* transliteration;

	static int count;
public:
	Person() : Person(generateRandomID(), "NoName", "NoSurname", "N/A") {}

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

	~Person() {
		delete[] identification;
		delete[] name;
		delete[] surname;
		delete[] transliteration;
		count--;
	}

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

	void GetInfo() {
		cout << "ID: " << identification << '\n';
		cout << "Name: " << name << '\n';
		cout << "Surname: " << surname << '\n';
		cout << "Transliteration: " << transliteration << '\n';
		showDate();
		cout << endl;
	}

	static int GetCount() {
		return count;
	}
};

int Person::count = 0;

int main() {
	Person p1;
	p1.SetName("Igor").SetSurname("Shevchenko").SetTransliteration("Oleksandrovych");

	Person p2(p1);
	Person p3(p2);
	Person p4(p3);

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

