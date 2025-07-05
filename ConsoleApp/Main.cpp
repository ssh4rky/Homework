#include <iostream>

using namespace std;

class Student {
private:
    char* name;
    char* groupName;
    int age;

    int id;
    static int nextId;
public:
    Student() : Student(nullptr, nullptr, 0) {}

    Student(const char* name, const char* group, int age) : age(age) {
        this->id = nextId;
        nextId++;

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
        if (name) delete[] name;
        if (groupName) delete[] groupName;
    }

    Student& SetName(const char* name) {
        if (this->name) delete[] this->name;
        if (name == nullptr) {
            this->name = nullptr;
        }
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

    void Print();
};

int Student::nextId = 1;

void Student::Print()
{
    cout << "Name: " << name << '\n';
    cout << "Group: " << groupName << '\n';
    cout << "Age: " << age << '\n';
    cout << "ID: " << id << '\n';
}

int main()
{

    Student st1, st2, st3;

    st1.SetAge(18).SetName("Igor").SetGroupName("P410");
    st2.SetAge(17).SetName("Elena").SetGroupName("P410");
    st3.SetAge(19).SetName("Kirylo").SetGroupName("P410");

    st1.Print();
    st2.Print();
    st3.Print();
}