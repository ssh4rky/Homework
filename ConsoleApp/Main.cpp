#include <iostream>
#include <cstring>
#include <Windows.h>

// Константні члени класу
// Explicit конструктор

using namespace std;

//class User {
//private:
//    char* name;
//    int age;
//    static int userCount;
//public:
//    User(const char* name) {
//        this->name = new char[strlen(name) + 1];
//        strcpy_s(this->name, strlen(name) + 1, name);
//        this->age = 10;
//        userCount++;
//    }
//    User(const User& other) {
//        // Поверхневе копіювання
//        //this->name = other.name;
//        
//        this->name = new char[strlen(other.name) + 1];
//        strcpy_s(this->name, strlen(other.name) + 1, other.name);
//    }
//    ~User() {
//        delete[] name;
//    }
//    static int GetUserCount() {
//        //this->age;
//        return userCount;
//    }
//    int GetAge() {
//        return this->age;
//    }
//};
//
//int User::userCount = 0;

enum Breed {
    Sphynx, MainCoon, Siamese
};

class Cat {
private:
    char* name;
    int age;
    const Breed breed = MainCoon;
    static int count;
    mutable int test;
public:
    Cat(int breed) : Cat(nullptr, -1, (Breed)breed) {}
    Cat(Breed breed) : Cat(nullptr, -1, breed) {}
    Cat(const char* name, int age, Breed breed) : age(age), breed(breed) {
        if (name)
        {
            this->name = new char[strlen(name) + 1];
            strcpy_s(this->name, strlen(name) + 1, name);
        }
        else
        {
            this->name = nullptr;
        }

        test = 10;

        count++;
    }
    ~Cat() {
        if (this->name) delete[] this->name;
    }

    static int GetCount() {
        return count;
    }

    char* GetName() const {
        return name;
    }
    int GetAge() const {
        return age;
    }
    Breed GetBreed() const {
        return breed;
    }

    // Константний метод
    // ВСІ МЕТОДИ які не змінюють ВНУТРІШНІЙ СТАН ОБ'ЄКТА повинні бути оголошені як КОНСТАНТНІ
    void Print() const {
        /*this = new Cat(Sphynx);
        this->age = 10;*/
        //count++;
        this->test++;

        cout << test << '\n';

        cout << "Ім'я: " << name << '\n' << "Вік: " << age << '\n';
        switch (breed) {
        case Sphynx:
            cout << "Порода: сфінкс\n"; break;
        case MainCoon:
            cout << "Порода: мейн кун\n"; break;
        case Siamese:
            cout << "Порода: сіамський кіт\n"; break;
        }
    }

    // const Cat* this; - константний покажчик, не можна переназначити на інший об'єкт, але міняти сам об'єкт - можна
    // const Cat* const this; - константний покажчик на константу - не можна переназначити, не можна змінити об'єкт
    Cat& SetAge(int age) {
        //this = new Cat(Sphynx);
        this->age = age;
        return *this;
    }

    Cat& SetName(const char* name) {
        if (name)
        {
            this->name = new char[strlen(name) + 1];
            strcpy_s(this->name, strlen(name) + 1, name);
        }
        else
        {
            this->name = nullptr;
        }
        return *this;
    }

    /*Cat& SetBreed(Breed breed) {
        this->breed = breed;

        return *this;
    }*/
};

int Cat::count = 0;

void EnterCat(Cat& cat) {
    cout << "Введіть ім'я кота: ";
    char* buffer = new char[50];
    cin.getline(buffer, 50);
    cat.SetName(buffer);
    cout << "Ввеідть вік: ";
    int temp;
    cin >> temp;
    cat.SetAge(temp);
}

int main()
{
    SetConsoleOutputCP(1251);

    Cat cat(2);

    cout << cat.GetBreed() << '\n';

    EnterCat(cat);

    cat.Print();

    //User u;

    //User u1("name");
    //User u2(u1);

    //Cat cat2(MainCoon);

    //Cat cat4{ "Ім'я", 10, Sphynx };

    //Cat cat1("name", 10, Sphynx);
    //cat1.Print();

    //// Константний об'єкт - внутрійшній стан не повинен змінюватись
    //const Cat catC("Моїсей", 5, MainCoon);

    //catC.Print();



    //cout << User::GetUserCount() << '\n';

    //User u4;

    //User u5(u4);

    //cout << User::GetUserCount() << '\n';

    //cout << u4.GetAge() << '\n';
    ////cout << User::GetAge() << '\n';
}
