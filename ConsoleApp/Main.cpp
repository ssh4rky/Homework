#include <iostream>
#include <cstring>
#include <Windows.h>

using namespace std;

class Book {
private:
    char* autor;
    char* name;
    char* center;
    int year;
    int pages;

public:
    explicit Book(const char* autor = "", const char* name = "", const char* center = "", int year = 0, int pages = 0) {
        this->autor = new char[strlen(autor) + 1];
        strcpy_s(this->autor, strlen(autor) + 1, autor);

        this->name = new char[strlen(name) + 1];
        strcpy_s(this->name, strlen(name) + 1, name);

        this->center = new char[strlen(center) + 1];
        strcpy_s(this->center, strlen(center) + 1, center);

        this->year = year;
        this->pages = pages;
    }

    Book(const Book& other) {
        autor = new char[strlen(other.autor) + 1];
        strcpy_s(autor, strlen(other.autor) + 1, other.autor);

        name = new char[strlen(other.name) + 1];
        strcpy_s(name, strlen(other.name) + 1, other.name);

        center = new char[strlen(other.center) + 1];
        strcpy_s(center, strlen(other.center) + 1, other.center);

        year = other.year;
        pages = other.pages;
    }

    ~Book() {
        delete[] autor;
        delete[] name;
        delete[] center;
    }

    void GetInfo() const {
        cout << (const char*)u8"Ім'я: " << name << "\n";
        cout << (const char*)u8"Автор: " << autor << "\n";
        cout << (const char*)u8"Видавництво: " << center << "\n";
        cout << (const char*)u8"Рік: " << year << "\n";
        cout << (const char*)u8"Сторінок: " << pages << "\n\n";
    }

    const char* GetAutor() const { return autor; }
    const char* GetCenter() const { return center; }
    int GetYear() const { return year; }
};

void PrintBooksByAuthor(Book books[], int size, const char* author) {
    cout << (const char*)u8"\nКниги автора \"" << author << "\":\n";
    for (int i = 0; i < size; ++i)
        if (strcmp(books[i].GetAutor(), author) == 0)
            books[i].GetInfo();
}

void PrintBooksByCenter(Book books[], int size, const char* center) {
    cout << (const char*)u8"\nКниги видавництва \"" << center << "\":\n";
    for (int i = 0; i < size; ++i)
        if (strcmp(books[i].GetCenter(), center) == 0)
            books[i].GetInfo();
}

void PrintBooksAfterYear(Book books[], int size, int year) {
    cout << (const char*)u8"\nКниги, видані після " << year << " року:\n";
    for (int i = 0; i < size; ++i)
        if (books[i].GetYear() > year)
            books[i].GetInfo();
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    const int size = 3;
    Book books[size] = {
        Book((const char*)u8"Кобилянська", (const char*)u8"Царівна", (const char*)u8"Літопис", 1895, 240),
        Book((const char*)u8"Франко", (const char*)u8"Захар Беркут", (const char*)u8"Львівська друкарня", 1883, 280),
        Book((const char*)u8"Кобилянська", (const char*)u8"В неділю рано зілля копала", (const char*)u8"Літопис", 1909, 190)
    };

    PrintBooksByAuthor(books, size, (const char*)u8"Кобилянська");
    PrintBooksByCenter(books, size, (const char*)u8"Літопис");
    PrintBooksAfterYear(books, size, 1900);

    return 0;
}
