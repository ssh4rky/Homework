#include <iostream>
#include <Windows.h>

using namespace std;

// Порушення інкапсуляції
// int nextId = 1;

class Student {
private:
    char* name;
    char* groupName;
    int age;

    int id; // унікальний ідентифікатор студента, для кожного об'єкта пвинен мати інше значення
    static int nextId; // статичне поле, яке належить класу і спільне для всіх об'єктів
public:
    Student() : Student(nullptr, nullptr, 0) {}

    Student(const char* name, const char* group, int age) : age(age) {
        //int newId = rand();
        //// перевірка чи був вже такий id;
        //this->id = newId;

        this->id = nextId;
        nextId++;

        cout << "Виклик конструктора з параметрами\n";

        if (name == nullptr) this->name = nullptr;
        else {
            this->name = new char[strlen(name) + 1];
            strcpy_s(this->name, strlen(name) + 1, name);
        }

        if (group == nullptr) groupName = nullptr;
        else {
            groupName = new char[strlen(group) + 1];
            strcpy_s(groupName, strlen(group) + 1, group);
        }
    }

    ~Student() {
        // if(name != nullptr) - варіант без перетворення типу
        if (name) delete[] name;
        if (groupName) delete[] groupName;
    }

    Student& SetName(const char* name) {
        // Очищаємо старе ім'я
        if (this->name) delete[] this->name;
        // Якщо нове ім'я - nullptr зберігаємо nullptr
        if (name == nullptr) {
            this->name = nullptr;
        }
        // зберігаємо нове ім'я
        else {
            this->name = new char[strlen(name) + 1];
            strcpy_s(this->name, strlen(name) + 1, name);
        }

        return *this;
    }
    Student& SetGroupName(const char* groupName) {
        if (this->groupName) delete[] this->groupName;
        if (groupName == nullptr) {
            this->groupName = nullptr;
        }
        else {
            this->groupName = new char[strlen(groupName) + 1];
            strcpy_s(this->groupName, strlen(groupName) + 1, groupName);
        }
        return *this;
    }
    Student& SetAge(int age) {
        this->age = age;
        return *this;
    }

    // Статичні методи - належать класу, а не об'єкту, використовуються для роботи зі статичними полями
    // не приймають параметр this
    static int GetNextId() {
        //this->age;
        return nextId;
    }

    void Print();
};

// Ініціалізація статичного поля
int Student::nextId = 1;

// Покажчик this - це неявний параметр, який передається в методи об'єкту при виклику та зберігає адресу самого об'єкту.
void Student::Print(/*const Student* this*/)
{
    //Student student("Temp", "test", -10);
    // this = &student; - this - константний покажчик, не можна переназначити

    cout << "Ім'я: " << name << '\n';
    cout << "Група: " << groupName << '\n';
    cout << "Вік: " << age << '\n';
    cout << "ID: " << id << '\n';

    cout << "Адреса об'єкту: " << this << '\n';
}

int main()
{
    SetConsoleOutputCP(1251);

    Student student;

    student.SetAge(15).SetName("Сергій").SetGroupName("П78");



    /* student.Print();

     student.SetAge(15);
     student.SetName("Сергій");
     student.SetGroupName("П78");*/

    Student st("Антон", "П565", 16);

    cout << Student::GetNextId() << '\n';

    // Через ім'я класу - рекомандовний варіант
    //cout << &(Student::nextId) << '\n';

    // Через ім'я об'єкту не рекомендується
    /*cout << &(student.nextId) << '\n';
    cout << &(st.nextId) << '\n';*/

    //Student::nextId = 1;

    // nextId = 1; 

    Student st2("Антон2", "П56", 17);

    // Print(&st); - неявно
    cout << &st << '\n';
    st.Print();
    // Print(&st2); - неявно
    st2.Print();

    cout << main << '\n';

    //cout << st.Print << '\n';
    //cout << st2.Print << '\n';
}

