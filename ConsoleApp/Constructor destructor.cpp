// testing_system.cpp
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

// -------------------- Utilities --------------------
static std::string GenerateSalt(size_t len = 16) {
    static std::mt19937_64 rng((unsigned)time(nullptr));
    static const char chars[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    std::uniform_int_distribution<int> dist(0, (int)(sizeof(chars) - 2));
    std::string s;
    s.reserve(len);
    for (size_t i = 0; i < len; ++i) s.push_back(chars[dist(rng)]);
    return s;
}

static std::string HashWithSalt(const std::string& pwd, const std::string& salt) {
    // Note: For production use, use bcrypt/Argon2. This is a simple improvement:
    size_t h = std::hash<std::string>{}(pwd + salt);
    char buf[64];
    snprintf(buf, sizeof(buf), "%zx", h);
    return std::string(buf);
}

// -------------------- Passport --------------------
class Passport {
protected:
    string name, surname, patronymic, adress, phonenumber;
public:
    Passport() {};

    void SetInfo() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Name: "; getline(cin, name);
        cout << "Surname: "; getline(cin, surname);
        cout << "Patronymic: "; getline(cin, patronymic);
        cout << "Address: "; getline(cin, adress);
        cout << "PhoneNumber: "; getline(cin, phonenumber);
    }

    string GetName() const { return name; }
    string GetSurname() const { return surname; }
    string GetPatronymic() const { return patronymic; }
    string GetPhoneNumber() const { return phonenumber; }
    string GetAdress() const { return adress; }

    void GetDBInfo() const {
        cout << "Name: " << name << "\nSurname: " << surname << "\nPatronymic: " << patronymic
            << "\nAddress: " << adress << "\nPhone Number: " << phonenumber << "\n";
    }
};

// -------------------- Admin (base user DB ops) --------------------
class Admin {
protected:
    Passport passport;
    string username;
    string passwordHash;
    string salt;
    sqlite3* db{ nullptr };
    char* errMsg{ nullptr };
    bool isStudent = false;
public:
    Admin() = default;

    void OpenDB(const string& filename = "users.db") {
        int rc = sqlite3_open(filename.c_str(), &db);
        if (rc) throw runtime_error("Cannot open DB");

        // create users table with salt column
        const char* users_sql =
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE, "
            "password_hash TEXT, "
            "salt TEXT, "
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
            throw runtime_error("SQL error creating info table: " + msg);
        }
    }

    Passport& GetPassport() { return passport; }
    const Passport& GetPassport() const { return passport; }

    void CloseDB() {
        if (db) sqlite3_close(db);
        db = nullptr;
    }

    virtual ~Admin() { CloseDB(); }

    bool UsernameExists(const string& uname) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT COUNT(*) FROM users WHERE username = ?;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare UsernameExists failed");

        sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        bool exists = false;
        if (rc == SQLITE_ROW) exists = (sqlite3_column_int(stmt, 0) > 0);
        sqlite3_finalize(stmt);
        return exists;
    }

    // Generic register that can be used for admin and students (is_student param)
    long RegisterInternal(const string& uname, const string& pwd, int is_student_flag) {
        if (UsernameExists(uname)) throw runtime_error("Username already exists");

        username = uname;
        salt = GenerateSalt();
        passwordHash = HashWithSalt(pwd, salt);
        cout << "Fill info:\n";
        passport.SetInfo();

        sqlite3_stmt* stmt = nullptr;
        const char* insertUser = "INSERT INTO users (username, password_hash, salt, is_student) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertUser, -1, &stmt, nullptr) != SQLITE_OK)
            throw runtime_error("Prepare insert user failed");

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, is_student_flag);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert user failed (maybe username taken)");
        }
        sqlite3_finalize(stmt);

        long long userId = sqlite3_last_insert_rowid(db);

        const char* insertInfo = "INSERT INTO info (user_id, name, surname, patronymic, adress, phonenumber) VALUES (?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertInfo, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare insert info failed");

        sqlite3_bind_int64(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, passport.GetName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, passport.GetSurname().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, passport.GetPatronymic().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, passport.GetAdress().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, passport.GetPhoneNumber().c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert info failed");
        }
        sqlite3_finalize(stmt);

        cout << "Registered user '" << username << "' id=" << userId << " is_student=" << is_student_flag << "\n";
        return userId;
    }

    long Register(const string& uname, const string& pwd) {
        // by default register as admin (is_student = 0). Derived Student will override
        return RegisterInternal(uname, pwd, 0);
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

    long RegisterAdmin(const string& uname, const string& pwd) {
        if (AdminExists()) {
            throw runtime_error("Admin already exists! Only one admin can be created.");
        }
        return Register(uname, pwd);
    }

    // Check login for any user. Returns id and sets isStudent flag accordingly.
    long CheckLogin(const string& uname, const string& pwd) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, salt, password_hash, is_student FROM users WHERE username = ?;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare login failed");
        sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            long long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* salt_text = sqlite3_column_text(stmt, 1);
            const unsigned char* stored_hash_text = sqlite3_column_text(stmt, 2);
            int is_child = sqlite3_column_int(stmt, 3);

            string stored_salt = salt_text ? reinterpret_cast<const char*>(salt_text) : "";
            string stored_hash = stored_hash_text ? reinterpret_cast<const char*>(stored_hash_text) : "";

            sqlite3_finalize(stmt);

            string candidate_hash = HashWithSalt(pwd, stored_salt);
            if (candidate_hash == stored_hash) {
                this->isStudent = (is_child == 1);
                this->username = uname;
                cout << "Login success for " << uname << " id=" << id << "\n";
                return id;
            }
            else {
                cout << "Login failed (invalid credentials)\n";
                return -1;
            }
        }
        else {
            sqlite3_finalize(stmt);
            cout << "Login failed (user not found)\n";
            return -1;
        }
    }

    bool IsStudent() const { return isStudent; }

    // Admin-only: change admin credentials (username + password)
    void ChangeAdminCredentials(long adminId) {
        cout << "Change admin login/password\n";
        string newLogin, newPass;
        cout << "New login (leave empty to keep current): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, newLogin);
        cout << "New password (leave empty to keep current): ";
        getline(cin, newPass);

        if (newLogin.empty() && newPass.empty()) {
            cout << "Nothing changed.\n";
            return;
        }

        // Check uniqueness if login changed
        if (!newLogin.empty() && UsernameExists(newLogin)) {
            cout << "That login is already taken.\n";
            return;
        }

        if (!newLogin.empty()) {
            sqlite3_stmt* stmt = nullptr;
            const char* upd = "UPDATE users SET username = ? WHERE id = ?;";
            if (sqlite3_prepare_v2(db, upd, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare update login failed");
            sqlite3_bind_text(stmt, 1, newLogin.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, adminId);
            int rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) cout << "Failed to update login\n";
            else cout << "Login updated.\n";
        }

        if (!newPass.empty()) {
            string newSalt = GenerateSalt();
            string newHash = HashWithSalt(newPass, newSalt);
            sqlite3_stmt* stmt = nullptr;
            const char* upd = "UPDATE users SET password_hash = ?, salt = ? WHERE id = ?;";
            if (sqlite3_prepare_v2(db, upd, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare update password failed");
            sqlite3_bind_text(stmt, 1, newHash.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, newSalt.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, adminId);
            int rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) cout << "Failed to update password\n";
            else cout << "Password updated.\n";
        }
    }

    // Admin listing users (basic)
    void ListUsers() {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT u.id, u.username, u.is_student, i.name, i.surname FROM users u LEFT JOIN info i ON u.id = i.user_id ORDER BY u.id;";
        if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare list users failed");

        cout << left << setw(6) << "id" << setw(20) << "username" << setw(10) << "role" << setw(20) << "Name" << "\n";
        cout << string(60, '-') << "\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* uname = sqlite3_column_text(stmt, 1);
            int is_student = sqlite3_column_int(stmt, 2);
            const unsigned char* name = sqlite3_column_text(stmt, 3);
            const unsigned char* surname = sqlite3_column_text(stmt, 4);
            cout << setw(6) << id
                 << setw(20) << (uname ? reinterpret_cast<const char*>(uname) : "")
                 << setw(10) << (is_student ? "student" : "admin")
                 << setw(20) << (name ? reinterpret_cast<const char*>(name) : "")
                 << " " << (surname ? reinterpret_cast<const char*>(surname) : "") << "\n";
        }
        sqlite3_finalize(stmt);
    }

    // Admin delete user by id
    void DeleteUserById(long id) {
        sqlite3_stmt* stmt = nullptr;
        const char* delInfo = "DELETE FROM info WHERE user_id = ?;";
        if (sqlite3_prepare_v2(db, delInfo, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare delete info failed");
        sqlite3_bind_int64(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        const char* delUser = "DELETE FROM users WHERE id = ?;";
        if (sqlite3_prepare_v2(db, delUser, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare delete user failed");
        sqlite3_bind_int64(stmt, 1, id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc == SQLITE_DONE) cout << "Deleted user " << id << "\n";
        else cout << "Failed to delete user " << id << "\n";
    }
};

// -------------------- Student (derived) --------------------
class Student : public Admin {
public:
    Student() { isStudent = true; }

    long Register(const string& uname, const string& pwd) {
        return RegisterInternal(uname, pwd, 1);
    }

    long CheckLogin(const string& uname, const string& pwd) {
        long id = Admin::CheckLogin(uname, pwd);
        if (id != -1 && !isStudent) {
            // logged in but not a student
            cout << "This account is not a student.\n";
            return -1;
        }
        return id;
    }
};

// -------------------- Main --------------------
int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    try {
        Admin admin;
        admin.OpenDB();

        cout << "=== Welcome to Testing Program ===\n";

        bool quit = false;
        while (!quit) {
            cout << "\nMain menu:\n";
            cout << "1) Register\n";
            cout << "2) Login\n";
            cout << "3) Exit\n";
            cout << "Your choice: ";
            int mainChoice;
            if (!(cin >> mainChoice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input\n";
                continue;
            }

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
                        string login, pass;
                        cout << "Enter admin login: ";
                        cin >> login;
                        cout << "Enter admin password: ";
                        cin >> pass;
                        try {
                            admin.RegisterAdmin(login, pass);
                            cout << "\nAdmin registered successfully!\n";
                        }
                        catch (const exception& ex) {
                            cout << "Error: " << ex.what() << "\n";
                        }
                    }
                    else if (regChoice == 2) {
                        Student st;
                        st.OpenDB();
                        string login, pass;
                        cout << "Enter student login: ";
                        cin >> login;
                        cout << "Enter student password: ";
                        cin >> pass;
                        try {
                            st.Register(login, pass);
                            cout << "\nStudent registered successfully!\n";
                        }
                        catch (const exception& ex) {
                            cout << "Error: " << ex.what() << "\n";
                        }
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
                    string login, pass;
                    cout << "Enter student login: ";
                    cin >> login;
                    cout << "Enter student password: ";
                    cin >> pass;
                    try {
                        st.Register(login, pass);
                        cout << "\nStudent registered successfully!\n";
                    }
                    catch (const exception& ex) {
                        cout << "Error: " << ex.what() << "\n";
                    }
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
                    // student menu (simple)
                    bool slogout = false;
                    while (!slogout) {
                        cout << "\nStudent menu:\n1) View profile\n2) Logout\nChoice: ";
                        int ch; cin >> ch;
                        if (ch == 1) {
                            sqlite3_stmt* stmt = nullptr;
                            const char* q = "SELECT name, surname, patronymic, adress, phonenumber FROM info WHERE user_id = ?;";
                            if (sqlite3_prepare_v2(admin.db, q, -1, &stmt, nullptr) == SQLITE_OK) {
                                sqlite3_bind_int64(stmt, 1, id);
                                if (sqlite3_step(stmt) == SQLITE_ROW) {
                                    const unsigned char* name = sqlite3_column_text(stmt, 0);
                                    const unsigned char* surname = sqlite3_column_text(stmt, 1);
                                    const unsigned char* patr = sqlite3_column_text(stmt, 2);
                                    const unsigned char* addr = sqlite3_column_text(stmt, 3);
                                    const unsigned char* phone = sqlite3_column_text(stmt, 4);
                                    cout << "Name: " << (name ? reinterpret_cast<const char*>(name) : "") << "\n";
                                    cout << "Surname: " << (surname ? reinterpret_cast<const char*>(surname) : "") << "\n";
                                    cout << "Patronymic: " << (patr ? reinterpret_cast<const char*>(patr) : "") << "\n";
                                    cout << "Address: " << (addr ? reinterpret_cast<const char*>(addr) : "") << "\n";
                                    cout << "Phone: " << (phone ? reinterpret_cast<const char*>(phone) : "") << "\n";
                                }
                                else cout << "No profile info found.\n";
                                sqlite3_finalize(stmt);
                            }
                            else cout << "Failed to read profile.\n";
                        }
                        else if (ch == 2) {
                            slogout = true;
                            cout << "Logged out.\n";
                        }
                        else cout << "Invalid choice.\n";
                    }
                }
                else {
                    cout << "\nWelcome, admin " << login << "!\n";
                    // admin menu
                    bool alogout = false;
                    while (!alogout) {
                        cout << "\nAdmin menu:\n1) List users\n2) Delete user by id\n3) Change my credentials\n4) Logout\nChoice: ";
                        int ch; cin >> ch;
                        if (ch == 1) {
                            admin.ListUsers();
                        }
                        else if (ch == 2) {
                            cout << "Enter user id to delete: ";
                            long uid; cin >> uid;
                            if (uid <= 0) cout << "Invalid id\n";
                            else admin.DeleteUserById(uid);
                        }
                        else if (ch == 3) {
                            admin.ChangeAdminCredentials(id);
                        }
                        else if (ch == 4) {
                            alogout = true;
                            cout << "Logged out.\n";
                        }
                        else cout << "Invalid choice.\n";
                    }
                }
            }
            else if (mainChoice == 3) {
                quit = true;
            }
            else {
                cout << "Invalid choice.\n";
            }
        }

        admin.CloseDB();
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
    return 0;
}
