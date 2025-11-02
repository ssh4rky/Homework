#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <functional>
#include <windows.h>
#include "sqlite3.h"

using namespace std;

// Passport
class Passport {
protected:
    string name, surname, patronymic, adress, phonenumber;
public:
    Passport() {};

    void SetInfo() {
        cout << "Name: "; cin >> name;
        cout << "Surname: "; cin >> surname;
        cout << "Patronymic: "; cin >> patronymic;
        cout << "Adress: "; cin >> adress;
        cout << "PhoneNumber: "; cin >> phonenumber;
    }

    string GetName() const { return name; }
    string GetSurname() const { return surname; }
    string GetPatronymic() const { return patronymic; }
    string GetPhoneNumber() const { return phonenumber; }
    string GetAdress() const { return adress; }

    void GetDBInfo() const {
        cout << "Name: " << name << "\nSurname: " << surname << "\nPatronymic: " << patronymic
            << "\nAdress: " << adress << "\n"
            << "\nPhone Number: " << phonenumber << "\n";
    }
};

// Admin
class Admin {
protected:
    Passport passport;
    string username;
    string passwordHash;
    sqlite3* db{ nullptr };
    char* errMsg{ nullptr };
    bool isStudent = false;
public:
    Admin() = default;

    static string HashPassword(const string& pwd) {
        size_t h = hash<string>{}(pwd);
        char buf[32];
        snprintf(buf, sizeof(buf), "%zx", h);
        return string(buf);
    }

    void OpenDB(const string& filename = "users.db") {
        int rc = sqlite3_open(filename.c_str(), &db);
        if (rc) throw runtime_error("Cannot open DB");

        char* errMsg = nullptr;

        const char* users_sql =
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE, "
            "password_hash TEXT, "
            "is_student INTEGER DEFAULT 0);";

        rc = sqlite3_exec(db, users_sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            string msg = errMsg ? errMsg : "unknown";
            sqlite3_free(errMsg);
            throw runtime_error("SQL error creating users table: " + msg);
        }

        const char* info_sql =
            "CREATE TABLE IF NOT EXISTS info ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER, "
            "name TEXT, "
            "surname TEXT, "
            "patronymic TEXT, "
            "adress TEXT, "
            "phonenumber TEXT, "
            "FOREIGN KEY(user_id) REFERENCES users(id));";

        rc = sqlite3_exec(db, info_sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            string msg = errMsg ? errMsg : "unknown";
            sqlite3_free(errMsg);
            throw runtime_error("SQL error creating passports table: " + msg);
        }
    }

    Passport& GetPassport() { return passport; }
    const Passport& GetPassport() const { return passport; }

    void CloseDB() {
        if (db) sqlite3_close(db);
        db = nullptr;
    }

    virtual ~Admin() { CloseDB(); }

    long Register() {
        cout << "Fill info:\n";
        cout << "Enter username: "; cin >> username;
        string password;
        cout << "Enter password: "; cin >> password;
        passwordHash = HashPassword(password);
        passport.SetInfo();

        int is_student = false;
        this->isStudent = (is_student == 1);

        sqlite3_stmt* stmt = nullptr;
        const char* insertUser = "INSERT INTO users (username, password_hash, is_student) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertUser, -1, &stmt, nullptr) != SQLITE_OK) {
            throw runtime_error("Prepare insert user failed");
        }
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, is_student);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert user failed (maybe username taken)");
        }
        sqlite3_finalize(stmt);

        long long userId = sqlite3_last_insert_rowid(db);

        const char* insertInfo = "INSERT INTO info (user_id, name, surname, patronymic, adress, phonenumber) VALUES (?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertInfo, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare insert passport failed");
        sqlite3_bind_int64(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, passport.GetName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, passport.GetSurname().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, passport.GetPatronymic().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, passport.GetAdress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, passport.GetPhoneNumber().c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert passport failed");
        }
        sqlite3_finalize(stmt);

        cout << "Registered Admin '" << username << "' id=" << userId << "\n";
        return userId;
    }

    bool IsStudent() const { return isStudent; }

    long CheckLogin(const string& uname, const string& pwd) {
        username = uname;
        passwordHash = HashPassword(pwd);
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, is_student FROM users WHERE username = ? AND password_hash = ?;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare login failed");
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            long long id = sqlite3_column_int64(stmt, 0);
            int is_child = sqlite3_column_int(stmt, 1);
            this->isStudent = (is_child == 1);
            sqlite3_finalize(stmt);
            cout << "Login success for " << username << " id=" << id << "\n";
            return id;
        }
        else {
            sqlite3_finalize(stmt);
            cout << "Login failed\n";
            return -1;
        }
    }

    bool AdminExists() {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT COUNT(*) FROM users WHERE is_student = 0;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare check admin failed");

        int rc = sqlite3_step(stmt);
        bool exists = false;
        if (rc == SQLITE_ROW) {
            exists = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
        return exists;
    }

    long RegisterAdmin() {
        if (AdminExists()) {
            throw runtime_error("Admin already exists! Only one admin can be created.");
        }
        return Register();
    }
};

// Student
class Student : public Admin {
public:
    Student() {
        isStudent = true;
    }

    long Register() {
        cout << "Fill info:\n";
        cout << "Enter username: "; cin >> username;
        string password;
        cout << "Enter password: "; cin >> password;
        passwordHash = HashPassword(password);
        passport.SetInfo();

        sqlite3_stmt* stmt = nullptr;
        const char* insertUser =
            "INSERT INTO users (username, password_hash, is_student) VALUES (?, ?, 1);";

        if (sqlite3_prepare_v2(db, insertUser, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare insert user failed");

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert student failed (username may already exist)");
        }
        sqlite3_finalize(stmt);

        long long userId = sqlite3_last_insert_rowid(db);

        const char* insertInfo =
            "INSERT INTO info (user_id, name, surname, patronymic, adress, phonenumber) VALUES (?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertInfo, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare insert passport failed");

        sqlite3_bind_int64(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, passport.GetName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, passport.GetSurname().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, passport.GetPatronymic().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, passport.GetAdress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, passport.GetPhoneNumber().c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert passport failed");
        }
        sqlite3_finalize(stmt);

        cout << "Registered student '" << username << "' id=" << userId << "\n";
        return userId;
    }

    long CheckLogin(const string& uname, const string& pwd) {
        username = uname;
        passwordHash = HashPassword(pwd);

        sqlite3_stmt* stmt = nullptr;
        const char* q =
            "SELECT id FROM users WHERE username = ? AND password_hash = ? AND is_student = 1;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare login failed");

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            long long id = sqlite3_column_int64(stmt, 0);
            sqlite3_finalize(stmt);
            cout << "Student login success for " << username << " (id=" << id << ")\n";
            return id;
        }
        else {
            sqlite3_finalize(stmt);
            cout << "Login failed (invalid credentials)\n";
            return -1;
        }
    }
};


int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    try {
        Admin admin;
        admin.OpenDB();

        cout << "=== Welcome to Testing Program ===\n";
        cout << "1) Register\n";
        cout << "2) Login\n";
        cout << "Your choice: ";

        int mainChoice;
        cin >> mainChoice;
        cout << "\n";

        if (mainChoice == 1) {
            bool adminExists = admin.AdminExists();

            if (!adminExists) {
                cout << "No admin found yet.\n";
                cout << "Choose who to register:\n";
                cout << "1) Admin\n";
                cout << "2) Student\n";
                cout << "Your choice: ";
                int regChoice;
                cin >> regChoice;

                if (regChoice == 1) {
                    admin.RegisterAdmin();
                    cout << "\nAdmin registered successfully!\n";
                }
                else if (regChoice == 2) {
                    Student st;
                    st.OpenDB();
                    st.Register();
                    cout << "\nStudent registered successfully!\n";
                    st.CloseDB();
                }
                else {
                    cout << "Invalid choice.\n";
                }
            }
            else {
                cout << "Admin already exists. Only student registration available.\n";
                Student st;
                st.OpenDB();
                st.Register();
                cout << "\nStudent registered successfully!\n";
                st.CloseDB();
            }
        }
        else if (mainChoice == 2) {
            string login, pass;
            cout << "Enter login: ";
            cin >> login;
            cout << "Enter password: ";
            cin >> pass;

            long id = admin.CheckLogin(login, pass);
            if (id == -1) {
                cout << "Invalid credentials.\n";
            }
            else if (admin.IsStudent()) {
                cout << "\nWelcome, student " << login << "!\n";
            }
            else {
                cout << "\nWelcome, admin " << login << "!\n";
            }
        }
        else {
            cout << "Invalid choice.\n";
        }

        admin.CloseDB();
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
}
