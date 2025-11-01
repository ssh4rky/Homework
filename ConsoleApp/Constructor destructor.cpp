#include <iostream>
#include <windows.h>
#include <cstring>
using namespace std;

class Student {
private:
    char* name;
    int student_id;
    static int student_count;
public:
    Student() {
        name = new char[1];
        name[0] = '\0';
        student_id = 0;
    }

    Student(const char* n) {
        int len = strlen(n) + 1;
        name = new char[len];
        strcpy_s(name, len, n);
        student_id = ++student_count;
    }

    Student(const Student& other) {
        int len = strlen(other.name) + 1;
        name = new char[len];
        strcpy_s(name, len, other.name);
        student_id = other.student_id;
    }

    Student& operator=(const Student& other) {
        if (this != &other) {
            delete[] name;
            int len = strlen(other.name) + 1;
            name = new char[len];
            strcpy_s(name, len, other.name);
            student_id = other.student_id;
        }
        return *this;
    }

    ~Student() {
        delete[] name;
    }

    const char* GetName() const { return name; }
    int GetId() const { return student_id; }

    void Print() const {
        cout << "Name: " << name << ", ID: " << student_id << endl;
    }
};

int Student::student_count = 0;

class Group {
private:
    char* name;
    Student* students;
    int count;

public:
    Group(const char* n = "Unnamed") {
        int len = strlen(n) + 1;
        name = new char[len];
        strcpy_s(name, len, n);
        students = nullptr;
        count = 0;
    }

    Group(const Group& other) {
        int len = strlen(other.name) + 1;
        name = new char[len];
        strcpy_s(name, len, other.name);

        count = other.count;
        students = new Student[count];
        for (int i = 0; i < count; i++)
            students[i] = other.students[i];
    }

    Group& operator=(const Group& other) {
        if (this != &other) {
            delete[] name;
            delete[] students;

            int len = strlen(other.name) + 1;
            name = new char[len];
            strcpy_s(name, len, other.name);

            count = other.count;
            students = new Student[count];
            for (int i = 0; i < count; i++)
                students[i] = other.students[i];
        }
        return *this;
    }

    ~Group() {
        delete[] name;
        delete[] students;
    }

    void AddStudent(const Student& s) {
        Student* newArr = new Student[count + 1];
        for (int i = 0; i < count; i++)
            newArr[i] = students[i];

        newArr[count] = s;
        delete[] students;
        students = newArr;
        count++;
    }

    void RemoveStudent(int id) {
        if (count == 0) {
            cout << "The group is empty!" << endl;
            return;
        }

        int index = -1;
        for (int i = 0; i < count; i++) {
            if (students[i].GetId() == id) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            cout << "No student found with that ID!" << endl;
            return;
        }

        Student* newArr = new Student[count - 1];
        for (int i = 0, j = 0; i < count; i++) {
            if (i != index)
                newArr[j++] = students[i];
        }

        delete[] students;
        students = newArr;
        count--;
        cout << "Student removed successfully." << endl;
    }

    void ShowStudents() const {
        cout << "\n=== Student list for group \"" << name << "\" ===" << endl;
        if (count == 0) {
            cout << "No students in the group." << endl;
            return;
        }

        for (int i = 0; i < count; i++) {
            cout << i + 1 << ". ";
            students[i].Print();
        }
    }

    const char* GetName() const { return name; }
    int GetCount() const { return count; }
};

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    Group* group = nullptr;
    int choice;

    do {
        cout << "\n===== MENU =====" << endl;
        cout << "1. Create a new group" << endl;
        cout << "2. Add a student" << endl;
        cout << "3. Remove a student" << endl;
        cout << "4. Show all students" << endl;
        cout << "0. Exit" << endl;
        cout << "Your choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: {
            char groupName[50];
            cout << "Enter group name: ";
            cin.getline(groupName, 50);
            delete group;
            group = new Group(groupName);
            cout << "Group \"" << group->GetName() << "\" created successfully!" << endl;
            break;
        }
        case 2: {
            if (!group) {
                cout << "Please create a group first!" << endl;
                break;
            }
            char name[50];
            cout << "Enter student name: ";
            cin.getline(name, 50);
            group->AddStudent(Student(name));
            cout << "Student added successfully!" << endl;
            break;
        }
        case 3: {
            if (!group) {
                cout << "Please create a group first!" << endl;
                break;
            }
            int id;
            cout << "Enter student ID to remove: ";
            cin >> id;
            cin.ignore();
            group->RemoveStudent(id);
            break;
        }
        case 4: {
            if (!group) {
                cout << "Please create a group first!" << endl;
                break;
            }
            group->ShowStudents();
            break;
        }
        case 0:
            cout << "Exiting program..." << endl;
            break;
        default:
            cout << "Invalid choice!" << endl;
        }
    } while (choice != 0);

    delete group;
    return 0;
}
