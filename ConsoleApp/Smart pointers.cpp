#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Student {
private:
    string name;
    int student_id;
    static int student_count;

public:
    Student() : name(""), student_id(0) {}

    Student(const string& n) : name(n) {
        student_id = ++student_count;
    }

    Student(const Student& other) = default;
    Student& operator=(const Student& other) = default;

    ~Student() = default;

    const string& GetName() const { return name; }
    int GetId() const { return student_id; }

    void Print() const {
        cout << "Name: " << name << ", ID: " << student_id << endl;
    }
};

int Student::student_count = 0;


class Group {
private:
    string name;
    vector<Student> students;

public:
    Group(const string& n = "Unnamed") : name(n) {}

    void AddStudent(const Student& s) {
        students.push_back(s);
    }

    void RemoveStudent(int id) {
        if (students.empty()) {
            cout << "The group is empty!" << endl;
            return;
        }

        auto it = find_if(students.begin(), students.end(),
            [id](const Student& s) { return s.GetId() == id; });

        if (it == students.end()) {
            cout << "No student found with that ID!" << endl;
            return;
        }

        students.erase(it);
        cout << "Student removed successfully." << endl;
    }

    void ShowStudents() const {
        cout << "\n=== Student list for group \"" << name << "\" ===" << endl;

        if (students.empty()) {
            cout << "No students in this group." << endl;
            return;
        }

        for (size_t i = 0; i < students.size(); ++i) {
            cout << i + 1 << ". ";
            students[i].Print();
        }
    }

    const string& GetName() const { return name; }
    int GetCount() const { return static_cast<int>(students.size()); }
};


int main() {
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
            string groupName;
            cout << "Enter group name: ";
            getline(cin, groupName);
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
            string name;
            cout << "Enter student name: ";
            getline(cin, name);
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
