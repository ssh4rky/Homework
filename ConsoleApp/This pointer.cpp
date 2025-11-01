#include <iostream>
#include <cstring>
using namespace std;

class Human {
    char* name;
    int age;

public:
    Human() : name(nullptr), age(0) {}

    Human(const char* n, int a) {
        name = new char[strlen(n) + 1];
        strcpy_s(name, strlen(n) + 1, n);
        age = a;
    }

    Human(const Human& other) {
        cout << "[Human] Copy constructor called\n";
        name = new char[strlen(other.name) + 1];
        strcpy_s(name, strlen(other.name) + 1, other.name);
        age = other.age;
    }

    ~Human() {
        delete[] name;
    }

    Human& operator=(const Human& other) {
        if (this == &other) return *this;

        delete[] name;
        name = new char[strlen(other.name) + 1];
        strcpy_s(name, strlen(other.name) + 1, other.name);
        age = other.age;

        return *this;
    }

    void Show() const {
        cout << "Name: " << name << ", Age: " << age << endl;
    }
};

class Flat {
    Human* people;
    int peopleCount;

public:
    Flat() : people(nullptr), peopleCount(0) {}

    Flat(Human* arr, int count) {
        peopleCount = count;
        people = new Human[peopleCount];
        for (int i = 0; i < peopleCount; i++)
            people[i] = arr[i];
    }

    Flat(const Flat& other) {
        cout << "[Flat] Copy constructor called\n";
        peopleCount = other.peopleCount;
        people = new Human[peopleCount];
        for (int i = 0; i < peopleCount; i++)
            people[i] = other.people[i];
    }

    ~Flat() {
        delete[] people;
    }

    void Show() const {
        cout << "Number of residents: " << peopleCount << endl;
        for (int i = 0; i < peopleCount; i++) {
            cout << "  ";
            people[i].Show();
        }
    }
};

class House {
    Flat* flats;
    int flatCount;

public:
    House() : flats(nullptr), flatCount(0) {}

    House(Flat* arr, int count) {
        flatCount = count;
        flats = new Flat[flatCount];
        for (int i = 0; i < flatCount; i++)
            flats[i] = arr[i];
    }

    House(const House& other) {
        cout << "[House] Copy constructor called\n";
        flatCount = other.flatCount;
        flats = new Flat[flatCount];
        for (int i = 0; i < flatCount; i++)
            flats[i] = other.flats[i];
    }

    ~House() {
        delete[] flats;
    }

    void Show() const {
        cout << "The house has " << flatCount << " flat(s):\n";
        for (int i = 0; i < flatCount; i++) {
            cout << "  Flat #" << i + 1 << ":\n";
            flats[i].Show();
        }
    }
};

int main() {
    Human p1("Barabulka", 30);
    Human p2("Mykyta", 25);
    Human p3("Black Settler", 40);

    Human flat1People[] = { p1, p2 };
    Human flat2People[] = { p3 };

    Flat f1(flat1People, 2);
    Flat f2(flat2People, 1);

    Flat flats[] = { f1, f2 };
    House h1(flats, 2);

    House h2 = h1;

    cout << "\n=== House 1 ===\n";
    h1.Show();

    cout << "\n=== House 2 (copy) ===\n";
    h2.Show();

    return 0;
}
