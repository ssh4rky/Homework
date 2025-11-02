#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <functional>
#include <sstream>
#include <map>
#include <cstdlib>
#include <windows.h>
#include "sqlite3.h"

using namespace std;

static string hashPassword(const string& pwd) {
    size_t h = hash<string>{}(pwd);
    char buf[32];
    snprintf(buf, sizeof(buf), "%zx", h);
    return string(buf);
}

static string nowStr() {
    time_t t = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return string(buf);
}

static void checkSqlite(int rc, char* err) {
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
        string msg = err ? err : "SQLite error";
        if (err) sqlite3_free(err);
        throw runtime_error(msg);
    }
}

struct SqlDB {
    sqlite3* db = nullptr;
    string filename;
    SqlDB() = default;
    ~SqlDB() { close(); }

    void open(const string& fn) {
        filename = fn;
        int rc = sqlite3_open(fn.c_str(), &db);
        if (rc) throw runtime_error("Cannot open DB: " + fn);
    }

    void close() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    void exec(const string& sql) {
        char* err = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
        if (rc != SQLITE_OK) {
            string em = err ? err : "unknown";
            if (err) sqlite3_free(err);
            throw runtime_error("SQL error: " + em + "\nWhile executing: " + sql);
        }
    }
};

struct Passport {
    string name, surname, patronymic, address, phone;
    void inputFromConsole() {
        cout << "Name: "; getline(cin, name);
        cout << "Surname: "; getline(cin, surname);
        cout << "Patronymic: "; getline(cin, patronymic);
        cout << "Address: "; getline(cin, address);
        cout << "Phone: "; getline(cin, phone);
    }
    void print() const {
        cout << "Name: " << name << "\nSurname: " << surname << "\nPatronymic: " << patronymic
            << "\nAddress: " << address << "\nPhone: " << phone << "\n";
    }
};

class UsersDB {
private:
    SqlDB s;
public:
    void open(const string& fn = "users.db") {
        s.open(fn);
        s.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "is_student INTEGER NOT NULL DEFAULT 1"
            ");"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS info ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "name TEXT, surname TEXT, patronymic TEXT, address TEXT, phone TEXT, "
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
            ");"
        );
    }

    void close() { s.close(); }

    bool adminExists() {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT COUNT(*) FROM users WHERE is_student = 0;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare adminExists failed");
        int rc = sqlite3_step(stmt);
        bool res = false;
        if (rc == SQLITE_ROW) res = sqlite3_column_int(stmt, 0) > 0;
        sqlite3_finalize(stmt);
        return res;
    }

    long createUser(const string& username, const string& passwordHash, bool isStudent, const Passport& p) {
        sqlite3_stmt* stmt = nullptr;
        const char* insUser = "INSERT INTO users (username, password_hash, is_student) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(s.db, insUser, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare insert user failed");
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, isStudent ? 1 : 0);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert user failed (username maybe taken)");
        }
        sqlite3_finalize(stmt);
        long long uid = sqlite3_last_insert_rowid(s.db);

        const char* insInfo = "INSERT INTO info (user_id, name, surname, patronymic, address, phone) VALUES (?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(s.db, insInfo, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare insert info failed");
        sqlite3_bind_int64(stmt, 1, uid);
        sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, p.surname.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, p.patronymic.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, p.address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, p.phone.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw runtime_error("Insert info failed");
        }
        sqlite3_finalize(stmt);
        return uid;
    }

    long checkLogin(const string& username, const string& passwordHash, bool requireStudent, bool& isStudentOut) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, is_student FROM users WHERE username = ? AND password_hash = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare login failed");
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            long long id = sqlite3_column_int64(stmt, 0);
            int isSt = sqlite3_column_int(stmt, 1);
            sqlite3_finalize(stmt);
            isStudentOut = (isSt == 1);
            if (requireStudent && !isStudentOut) return -1;
            return id;
        }
        sqlite3_finalize(stmt);
        return -1;
    }

    vector<pair<long, string>> listUsers(bool studentsOnly = true) {
        vector<pair<long, string>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = studentsOnly ? "SELECT id, username FROM users WHERE is_student = 1;" : "SELECT id, username FROM users;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare listUsers failed");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* un = sqlite3_column_text(stmt, 1);
            out.emplace_back(id, string((const char*)un));
        }
        sqlite3_finalize(stmt);
        return out;
    }

    void deleteUser(long userId) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "DELETE FROM users WHERE id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare deleteUser failed");
        sqlite3_bind_int64(stmt, 1, userId);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) throw runtime_error("Delete user failed");
    }

    Passport getInfo(long userId) {
        Passport p;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT name, surname, patronymic, address, phone FROM info WHERE user_id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare getInfo failed");
        sqlite3_bind_int64(stmt, 1, userId);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            p.name = (const char*)sqlite3_column_text(stmt, 0);
            p.surname = (const char*)sqlite3_column_text(stmt, 1);
            p.patronymic = (const char*)sqlite3_column_text(stmt, 2);
            p.address = (const char*)sqlite3_column_text(stmt, 3);
            p.phone = (const char*)sqlite3_column_text(stmt, 4);
        }
        sqlite3_finalize(stmt);
        return p;
    }

    void changePassword(long userId, const string& newHash) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "UPDATE users SET password_hash = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare changePassword failed");
        sqlite3_bind_text(stmt, 1, newHash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, userId);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) throw runtime_error("Change password failed");
    }

    void updateInfo(long userId, const Passport& p) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "UPDATE info SET name = ?, surname = ?, patronymic = ?, address = ?, phone = ? WHERE user_id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare updateInfo failed");
        sqlite3_bind_text(stmt, 1, p.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, p.surname.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, p.patronymic.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, p.address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, p.phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 6, userId);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) throw runtime_error("Update info failed");
    }
};

class TestsDB {
private:
    SqlDB s;
public:
    void open(const string& fn = "tests.db") {
        s.open(fn);
        s.exec(
            "CREATE TABLE IF NOT EXISTS categories ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL UNIQUE);"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS tests ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, category_id INTEGER, title TEXT NOT NULL, "
            "FOREIGN KEY(category_id) REFERENCES categories(id) ON DELETE CASCADE);"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS questions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, test_id INTEGER, text TEXT NOT NULL, "
            "FOREIGN KEY(test_id) REFERENCES tests(id) ON DELETE CASCADE);"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS choices ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, question_id INTEGER, text TEXT NOT NULL, is_correct INTEGER NOT NULL DEFAULT 0, "
            "FOREIGN KEY(question_id) REFERENCES questions(id) ON DELETE CASCADE);"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS results ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, test_id INTEGER, taken_at TEXT, "
            "correct INTEGER, total INTEGER, percent REAL, grade INTEGER);"
        );
        s.exec(
            "CREATE TABLE IF NOT EXISTS progress ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, test_id INTEGER, current_q INTEGER, "
            "answers TEXT, saved_at TEXT);"
        );
    }

    void close() { s.close(); }

    sqlite3* db() const { return s.db; }
    void clearProgress(long user_id, long test_id) {
        sqlite3_stmt* stmt = nullptr;
        const char* delq = "DELETE FROM progress WHERE user_id = ? AND test_id = ?;";
        if (sqlite3_prepare_v2(db(), delq, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, user_id);
            sqlite3_bind_int64(stmt, 2, test_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    long createCategory(const string& title) {
        sqlite3_stmt* stmt = nullptr;
        const char* ins = "INSERT INTO categories (title) VALUES (?);";
        if (sqlite3_prepare_v2(s.db, ins, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare createCategory failed");
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); throw runtime_error("Insert category failed"); }
        sqlite3_finalize(stmt);
        return sqlite3_last_insert_rowid(s.db);
    }

    long createTest(long category_id, const string& title) {
        sqlite3_stmt* stmt = nullptr;
        const char* ins = "INSERT INTO tests (category_id, title) VALUES (?, ?);";
        if (sqlite3_prepare_v2(s.db, ins, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare createTest failed");
        sqlite3_bind_int64(stmt, 1, category_id);
        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); throw runtime_error("Insert test failed"); }
        sqlite3_finalize(stmt);
        return sqlite3_last_insert_rowid(s.db);
    }

    long createQuestion(long test_id, const string& text) {
        sqlite3_stmt* stmt = nullptr;
        const char* ins = "INSERT INTO questions (test_id, text) VALUES (?, ?);";
        if (sqlite3_prepare_v2(s.db, ins, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare createQuestion failed");
        sqlite3_bind_int64(stmt, 1, test_id);
        sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); throw runtime_error("Insert question failed"); }
        sqlite3_finalize(stmt);
        return sqlite3_last_insert_rowid(s.db);
    }

    long createChoice(long question_id, const string& text, bool is_correct) {
        sqlite3_stmt* stmt = nullptr;
        const char* ins = "INSERT INTO choices (question_id, text, is_correct) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(s.db, ins, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare createChoice failed");
        sqlite3_bind_int64(stmt, 1, question_id);
        sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, is_correct ? 1 : 0);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); throw runtime_error("Insert choice failed"); }
        sqlite3_finalize(stmt);
        return sqlite3_last_insert_rowid(s.db);
    }

    vector<pair<long, string>> listCategories() {
        vector<pair<long, string>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, title FROM categories;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare listCategories failed");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* t = sqlite3_column_text(stmt, 1);
            out.emplace_back(id, string((const char*)t));
        }
        sqlite3_finalize(stmt);
        return out;
    }

    vector<pair<long, string>> listTests(long category_id) {
        vector<pair<long, string>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, title FROM tests WHERE category_id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare listTests failed");
        sqlite3_bind_int64(stmt, 1, category_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* t = sqlite3_column_text(stmt, 1);
            out.emplace_back(id, string((const char*)t));
        }
        sqlite3_finalize(stmt);
        return out;
    }

    struct Choice { long id = 0; string text; bool is_correct; };
    struct Question { long id = 0; string text; vector<Choice> choices; };

    vector<Question> getQuestions(long test_id) {
        vector<Question> qs;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, text FROM questions WHERE test_id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare getQuestions failed");
        sqlite3_bind_int64(stmt, 1, test_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Question qq;
            qq.id = sqlite3_column_int64(stmt, 0);
            qq.text = (const char*)sqlite3_column_text(stmt, 1);
            sqlite3_stmt* s2 = nullptr;
            const char* q2 = "SELECT id, text, is_correct FROM choices WHERE question_id = ?;";
            if (sqlite3_prepare_v2(s.db, q2, -1, &s2, nullptr) != SQLITE_OK) throw runtime_error("Prepare getChoices failed");
            sqlite3_bind_int64(s2, 1, qq.id);
            while (sqlite3_step(s2) == SQLITE_ROW) {
                Choice c;
                c.id = sqlite3_column_int64(s2, 0);
                c.text = (const char*)sqlite3_column_text(s2, 1);
                c.is_correct = sqlite3_column_int(s2, 2) == 1;
                qq.choices.push_back(c);
            }
            sqlite3_finalize(s2);
            qs.push_back(qq);
        }
        sqlite3_finalize(stmt);
        return qs;
    }

    void saveResult(long user_id, long test_id, int correct, int total, double percent, int grade) {
        sqlite3_stmt* stmt = nullptr;
        const char* ins = "INSERT INTO results (user_id, test_id, taken_at, correct, total, percent, grade) VALUES (?, ?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(s.db, ins, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare saveResult failed");
        sqlite3_bind_int64(stmt, 1, user_id);
        sqlite3_bind_int64(stmt, 2, test_id);
        sqlite3_bind_text(stmt, 3, nowStr().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, correct);
        sqlite3_bind_int(stmt, 5, total);
        sqlite3_bind_double(stmt, 6, percent);
        sqlite3_bind_int(stmt, 7, grade);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) throw runtime_error("Insert result failed");
    }

    vector<tuple<long, long, string, int, int, double, int>> getResultsForUser(long user_id) {
        vector<tuple<long, long, string, int, int, double, int>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT id, test_id, taken_at, correct, total, percent, grade FROM results WHERE user_id = ? ORDER BY taken_at DESC;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare getResultsForUser failed");
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            long test_id = sqlite3_column_int64(stmt, 1);
            string taken = (const char*)sqlite3_column_text(stmt, 2);
            int correct = sqlite3_column_int(stmt, 3);
            int total = sqlite3_column_int(stmt, 4);
            double percent = sqlite3_column_double(stmt, 5);
            int grade = sqlite3_column_int(stmt, 6);
            out.emplace_back(id, test_id, taken, correct, total, percent, grade);
        }
        sqlite3_finalize(stmt);
        return out;
    }

    void saveProgress(long user_id, long test_id, int current_q, const string& answers) {
        sqlite3_stmt* stmt = nullptr;
        const char* sel = "SELECT id FROM progress WHERE user_id = ? AND test_id = ?;";
        if (sqlite3_prepare_v2(s.db, sel, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare progress select failed");
        sqlite3_bind_int64(stmt, 1, user_id);
        sqlite3_bind_int64(stmt, 2, test_id);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            long id = sqlite3_column_int64(stmt, 0);
            sqlite3_finalize(stmt);
            sqlite3_stmt* upd = nullptr;
            const char* upq = "UPDATE progress SET current_q = ?, answers = ?, saved_at = ? WHERE id = ?;";
            if (sqlite3_prepare_v2(s.db, upq, -1, &upd, nullptr) != SQLITE_OK) throw runtime_error("Prepare progress update failed");
            sqlite3_bind_int(upd, 1, current_q);
            sqlite3_bind_text(upd, 2, answers.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(upd, 3, nowStr().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(upd, 4, id);
            int r2 = sqlite3_step(upd);
            sqlite3_finalize(upd);
            if (r2 != SQLITE_DONE) throw runtime_error("Update progress failed");
            return;
        }
        sqlite3_finalize(stmt);
        sqlite3_stmt* ins = nullptr;
        const char* inq = "INSERT INTO progress (user_id, test_id, current_q, answers, saved_at) VALUES (?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(s.db, inq, -1, &ins, nullptr) != SQLITE_OK) throw runtime_error("Prepare progress insert failed");
        sqlite3_bind_int64(ins, 1, user_id);
        sqlite3_bind_int64(ins, 2, test_id);
        sqlite3_bind_int(ins, 3, current_q);
        sqlite3_bind_text(ins, 4, answers.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(ins, 5, nowStr().c_str(), -1, SQLITE_TRANSIENT);
        int r3 = sqlite3_step(ins);
        sqlite3_finalize(ins);
        if (r3 != SQLITE_DONE) throw runtime_error("Insert progress failed");
    }

    bool loadProgress(long user_id, long test_id, int& current_q, string& answers) {
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT current_q, answers FROM progress WHERE user_id = ? AND test_id = ?;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare loadProgress failed");
        sqlite3_bind_int64(stmt, 1, user_id);
        sqlite3_bind_int64(stmt, 2, test_id);
        int rc = sqlite3_step(stmt);
        bool found = false;
        if (rc == SQLITE_ROW) {
            current_q = sqlite3_column_int(stmt, 0);
            const unsigned char* txt = sqlite3_column_text(stmt, 1);
            answers = txt ? (const char*)txt : "";
            found = true;
        }
        sqlite3_finalize(stmt);
        return found;
    }

    vector<pair<string, int>> statsResultsPerTest() {
        vector<pair<string, int>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT t.title, COUNT(r.id) FROM tests t LEFT JOIN results r ON t.id = r.test_id GROUP BY t.id;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare statsResultsPerTest failed");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string title = (const char*)sqlite3_column_text(stmt, 0);
            int cnt = sqlite3_column_int(stmt, 1);
            out.emplace_back(title, cnt);
        }
        sqlite3_finalize(stmt);
        return out;
    }

    vector<tuple<string, string, int, int, double>> statsResultsForTest(long test_id) {
        vector<tuple<string, string, int, int, double>> out;
        sqlite3_stmt* stmt = nullptr;
        const char* q = "SELECT u.username, r.taken_at, r.correct, r.total, r.percent FROM results r JOIN users u ON r.user_id = u.id WHERE r.test_id = ? ORDER BY r.taken_at DESC;";
        if (sqlite3_prepare_v2(s.db, q, -1, &stmt, nullptr) != SQLITE_OK) throw runtime_error("Prepare statsResultsForTest failed");
        sqlite3_bind_int64(stmt, 1, test_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string username = (const char*)sqlite3_column_text(stmt, 0);
            string taken_at = (const char*)sqlite3_column_text(stmt, 1);
            int correct = sqlite3_column_int(stmt, 2);
            int total = sqlite3_column_int(stmt, 3);
            double percent = sqlite3_column_double(stmt, 4);
            out.emplace_back(username, taken_at, correct, total, percent);
        }
        sqlite3_finalize(stmt);
        return out;
    }

    void exportAll(const string& filename) {
        ofstream out(filename);
        if (!out) throw runtime_error("Cannot open export file");
        auto cats = listCategories();
        for (auto& c : cats) {
            out << "CATEGORY|" << c.first << "|" << c.second << "\n";
            auto tests = listTests(c.first);
            for (auto& t : tests) {
                out << "TEST|" << t.first << "|" << t.second << "\n";
                auto qs = getQuestions(t.first);
                for (auto& q : qs) {
                    out << "QUESTION|" << q.id << "|" << q.text << "\n";
                    for (auto& ch : q.choices) {
                        out << "CHOICE|" << ch.id << "|" << (ch.is_correct ? "1" : "0") << "|" << ch.text << "\n";
                    }
                }
            }
        }
        out.close();
    }

    void importFromFile(const string& filename) {
        ifstream in(filename);
        if (!in) throw runtime_error("Cannot open import file");
        string line;
        long lastCategory = -1, lastTest = -1, lastQuestion = -1;
        while (getline(in, line)) {
            if (line.empty()) continue;
            vector<string> parts;
            string token;
            stringstream ss(line);
            while (getline(ss, token, '|')) parts.push_back(token);
            if (parts.size() < 2) continue;
            string type = parts[0];
            if (type == "CATEGORY" && parts.size() >= 3) {
                lastCategory = createCategory(parts[2]);
            }
            else if (type == "TEST" && parts.size() >= 3) {
                if (lastCategory == -1) throw runtime_error("TEST without CATEGORY in import");
                lastTest = createTest(lastCategory, parts[2]);
            }
            else if (type == "QUESTION" && parts.size() >= 3) {
                if (lastTest == -1) throw runtime_error("QUESTION without TEST in import");
                lastQuestion = createQuestion(lastTest, parts[2]);
            }
            else if (type == "CHOICE" && parts.size() >= 4) {
                if (lastQuestion == -1) throw runtime_error("CHOICE without QUESTION in import");
                bool is_corr = parts[2] == "1";
                createChoice(lastQuestion, parts[3], is_corr);
            }
        }
    }
};

static int percentToGrade(double percent) {
    int g = int(round((percent / 100.0) * 11.0)) + 1;
    if (g < 1) g = 1;
    if (g > 12) g = 12;
    return g;
}

class AdminUI {
    UsersDB& users;
    TestsDB& testsdb;
    long adminId;
public:
    AdminUI(UsersDB& u, TestsDB& t, long aid) : users(u), testsdb(t), adminId(aid) {}

    void run() {
        while (true) {
            cout << "\n--- Admin Menu ---\n";
            cout << "1) Manage users\n";
            cout << "2) Manage tests (categories/tests/questions)\n";
            cout << "3) View statistics\n";
            cout << "4) Export tests to file\n";
            cout << "5) Import tests from file\n";
            cout << "6) Change my password\n";
            cout << "0) Logout\n";
            cout << "Choice: ";
            int c; if (!(cin >> c)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string dummy; getline(cin, dummy);
            try {
                if (c == 1) manageUsers();
                else if (c == 2) manageTests();
                else if (c == 3) viewStats();
                else if (c == 4) exportTests();
                else if (c == 5) importTests();
                else if (c == 6) changeMyPassword();
                else if (c == 0) break;
                else cout << "Invalid choice\n";
            }
            catch (const exception& ex) {
                cout << "Error: " << ex.what() << "\n";
            }
        }
    }

private:
    void manageUsers() {
        while (true) {
            cout << "\n-- Manage Users --\n";
            cout << "1) List students\n";
            cout << "2) Create student\n";
            cout << "3) Delete student\n";
            cout << "4) Modify student info\n";
            cout << "0) Back\nChoice: ";
            int c; if (!(cin >> c)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string dummy; getline(cin, dummy);
            if (c == 1) {
                auto usersList = users.listUsers(true);
                for (auto& u : usersList) cout << u.first << ": " << u.second << "\n";
            }
            else if (c == 2) {
                Passport p;
                cout << "Enter new student info:\n";
                p.inputFromConsole();
                cout << "Username: "; string un; getline(cin, un);
                cout << "Password: "; string pw; getline(cin, pw);
                long uid = users.createUser(un, hashPassword(pw), true, p);
                cout << "Created student id=" << uid << "\n";
            }
            else if (c == 3) {
                cout << "Enter student id to delete: "; long id; cin >> id; string tmp; getline(cin, tmp);
                users.deleteUser(id);
                cout << "Deleted user " << id << "\n";
            }
            else if (c == 4) {
                cout << "Enter student id to modify: "; long id; cin >> id; string tmp; getline(cin, tmp);
                Passport p; cout << "Enter new info:\n"; p.inputFromConsole();
                users.updateInfo(id, p);
                cout << "Updated info\n";
            }
            else if (c == 0) break;
            else cout << "Invalid choice\n";
        }
    }

    void manageTests() {
        while (true) {
            cout << "\n-- Manage Tests --\n";
            cout << "1) Create category\n";
            cout << "2) Create test in category\n";
            cout << "3) Add question to test\n";
            cout << "4) Add choice to question\n";
            cout << "5) List categories & tests\n";
            cout << "0) Back\nChoice: ";
            int c; if (!(cin >> c)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string dummy; getline(cin, dummy);
            try {
                if (c == 1) {
                    cout << "Category title: "; string title; getline(cin, title);
                    long id = testsdb.createCategory(title);
                    cout << "Created category id=" << id << "\n";
                }
                else if (c == 2) {
                    auto cats = testsdb.listCategories();
                    if (cats.empty()) { cout << "No categories. Create one first.\n"; continue; }
                    cout << "Choose category id: \n";
                    for (auto& c0 : cats) cout << c0.first << ": " << c0.second << "\n";
                    cout << "id: "; long cid; cin >> cid; string tmp; getline(cin, tmp);
                    cout << "Test title: "; string ttitle; getline(cin, ttitle);
                    long tid = testsdb.createTest(cid, ttitle);
                    cout << "Test created id=" << tid << "\n";
                }
                else if (c == 3) {
                    cout << "Enter test id to add question to: "; long tid; cin >> tid; string tmp; getline(cin, tmp);
                    cout << "Question text: "; string qtxt; getline(cin, qtxt);
                    long qid = testsdb.createQuestion(tid, qtxt);
                    cout << "Question id=" << qid << "\n";
                }
                else if (c == 4) {
                    cout << "Enter question id to add choice to: "; long qid; cin >> qid; string tmp; getline(cin, tmp);
                    cout << "Choice text: "; string ctxt; getline(cin, ctxt);
                    cout << "Is correct? (1/0): "; int ic; cin >> ic; getline(cin, tmp);
                    long cid = testsdb.createChoice(qid, ctxt, ic == 1);
                    cout << "Choice id=" << cid << "\n";
                }
                else if (c == 5) {
                    auto cats = testsdb.listCategories();
                    for (auto& c0 : cats) {
                        cout << "Category " << c0.first << ": " << c0.second << "\n";
                        auto ts = testsdb.listTests(c0.first);
                        for (auto& t0 : ts) cout << "  Test " << t0.first << ": " << t0.second << "\n";
                    }
                }
                else if (c == 0) break;
                else cout << "Invalid choice\n";
            }
            catch (const exception& ex) {
                cout << "Error: " << ex.what() << "\n";
            }
        }
    }

    void viewStats() {
        while (true) {
            cout << "\n-- Stats --\n";
            cout << "1) Number of attempts per test\n";
            cout << "2) View attempts for specific test\n";
            cout << "3) Save stats to file\n";
            cout << "0) Back\nChoice: ";
            int c; if (!(cin >> c)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string dummy; getline(cin, dummy);
            if (c == 1) {
                auto v = testsdb.statsResultsPerTest();
                for (auto& p : v) cout << p.first << " -> attempts: " << p.second << "\n";
            }
            else if (c == 2) {
                cout << "Enter test id: "; long id; cin >> id; string tmp; getline(cin, tmp);
                auto v = testsdb.statsResultsForTest(id);
                for (auto& r : v) {
                    cout << get<0>(r) << " at " << get<1>(r) << " correct=" << get<2>(r) << "/" << get<3>(r) << " percent=" << get<4>(r) << "\n";
                }
            }
            else if (c == 3) {
                cout << "Filename to save stats: "; string fn; getline(cin, fn);
                ofstream out(fn);
                if (!out) { cout << "Cannot open file\n"; continue; }
                auto v = testsdb.statsResultsPerTest();
                out << "Attempts per test:\n";
                for (auto& p : v) out << p.first << " -> attempts: " << p.second << "\n";
                out.close();
                cout << "Saved\n";
            }
            else if (c == 0) break;
            else cout << "Invalid choice\n";
        }
    }

    void exportTests() {
        cout << "Enter filename to export to: "; string fn; getline(cin, fn);
        testsdb.exportAll(fn);
        cout << "Exported\n";
    }

    void importTests() {
        cout << "Enter filename to import from: "; string fn; getline(cin, fn);
        testsdb.importFromFile(fn);
        cout << "Imported\n";
    }

    void changeMyPassword() {
        cout << "Enter new password: "; string pw; getline(cin, pw);
        users.changePassword(adminId, hashPassword(pw));
        cout << "Password changed\n";
    }
};

class StudentUI {
    UsersDB& users;
    TestsDB& testsdb;
    long userId;
    string username;
public:
    StudentUI(UsersDB& u, TestsDB& t, long uid, const string& uname) : users(u), testsdb(t), userId(uid), username(uname) {}

    void run() {
        while (true) {
            cout << "\n--- Student Menu ---\n";
            cout << "1) View personal info\n";
            cout << "2) Edit personal info\n";
            cout << "3) View previous results\n";
            cout << "4) Take a test\n";
            cout << "5) Logout\n";
            cout << "Choice: ";
            int c; if (!(cin >> c)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string dummy; getline(cin, dummy);
            try {
                if (c == 1) viewInfo();
                else if (c == 2) editInfo();
                else if (c == 3) viewResults();
                else if (c == 4) takeTest();
                else if (c == 5) break;
                else cout << "Invalid choice\n";
            }
            catch (const exception& ex) {
                cout << "Error: " << ex.what() << "\n";
            }
        }
    }

private:
    void viewInfo() {
        auto p = users.getInfo(userId);
        p.print();
    }

    void editInfo() {
        Passport p;
        cout << "Enter new info:\n";
        p.inputFromConsole();
        users.updateInfo(userId, p);
        cout << "Updated\n";
    }

    void viewResults() {
        auto res = testsdb.getResultsForUser(userId);
        if (res.empty()) { cout << "No results yet\n"; return; }
        for (auto& r : res) {
            long id = get<0>(r);
            long test_id = get<1>(r);
            string taken_at = get<2>(r);
            int correct = get<3>(r);
            int total = get<4>(r);
            double percent = get<5>(r);
            int grade = get<6>(r);
            cout << "Test id " << test_id << " taken " << taken_at << " -> " << correct << "/" << total << " (" << fixed << setprecision(1) << percent << "%), grade=" << grade << "\n";
        }
    }

    void takeTest() {
        auto cats = testsdb.listCategories();
        if (cats.empty()) { cout << "No categories available, ask admin.\n"; return; }
        cout << "Categories:\n";
        for (auto& c : cats) cout << c.first << ": " << c.second << "\n";
        cout << "Enter category id: "; long cid; cin >> cid; string tmp; getline(cin, tmp);
        auto tests = testsdb.listTests(cid);
        if (tests.empty()) { cout << "No tests in this category\n"; return; }
        cout << "Tests:\n";
        for (auto& t : tests) cout << t.first << ": " << t.second << "\n";
        cout << "Enter test id: "; long tid; cin >> tid; getline(cin, tmp);

        auto qs = testsdb.getQuestions(tid);
        if (qs.empty()) { cout << "Test has no questions\n"; return; }

        int current_q = 0;
        string answers_serial;
        if (testsdb.loadProgress(userId, tid, current_q, answers_serial)) {
            cout << "Found saved progress. Continue? (y/n): ";
            char ch; cin >> ch; string rest; getline(cin, rest);
            if (ch == 'n' || ch == 'N') {
                current_q = 0;
                answers_serial.clear();
            }
        }

        map<long, long> answers;
        if (!answers_serial.empty()) {
            stringstream ss(answers_serial);
            string token;
            while (getline(ss, token, ';')) {
                if (token.empty()) continue;
                size_t pos = token.find(':');
                if (pos == string::npos) continue;
                long qid = stol(token.substr(0, pos));
                long cid = stol(token.substr(pos + 1));
                answers[qid] = cid;
            }
        }
        for (int i = current_q; i < (int)qs.size(); ++i) {
            auto& Q = qs[i];
            cout << "\nQuestion " << (i + 1) << " of " << qs.size() << ":\n";
            cout << Q.text << "\n";
            vector<TestsDB::Choice> choices = Q.choices;
            for (size_t j = 0; j < choices.size(); ++j) {
                cout << (j + 1) << ") " << choices[j].text << "\n";
            }
            cout << "Enter choice number (or 0 to pause and save progress): ";
            int ch; if (!(cin >> ch)) { cin.clear(); string tmp; getline(cin, tmp); --i; continue; }
            string tmp2; getline(cin, tmp2);
            if (ch == 0) {
                string ser;
                for (auto& kv : answers) ser += to_string(kv.first) + ":" + to_string(kv.second) + ";";
                testsdb.saveProgress(userId, tid, i, ser);
                cout << "Progress saved. You can resume later.\n";
                return;
            }
            if (ch < 1 || ch >(int)choices.size()) {
                cout << "Invalid choice, repeat question.\n";
                --i;
                continue;
            }
            long pickedChoiceId = choices[ch - 1].id;
            answers[Q.id] = pickedChoiceId;
        }

        int correct = 0;
        int total = (int)qs.size();
        for (auto& Q : qs) {
            long picked = 0;
            if (answers.count(Q.id)) picked = answers[Q.id];
            for (auto& c : Q.choices) {
                if (c.id == picked && c.is_correct) { correct++; break; }
            }
        }
        double percent = total == 0 ? 0.0 : (100.0 * correct / total);
        int grade = percentToGrade(percent);
        testsdb.saveResult(userId, tid, correct, total, percent, grade);
        testsdb.clearProgress(userId, tid);

        cout << "Test finished!\n";
        cout << "Correct: " << correct << "/" << total << "\n";
        cout << "Percent: " << fixed << setprecision(1) << percent << "%\n";
        cout << "Grade (12-point): " << grade << "\n";
    }
};

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    try {
        UsersDB users;
        TestsDB testsdb;
        users.open("users.db");
        testsdb.open("tests.db");

        cout << "=== Welcome to Testing Program ===\n";
        while (true) {
            cout << "\nMain menu:\n";
            cout << "1) Register\n";
            cout << "2) Login\n";
            cout << "0) Exit\n";
            cout << "Choice: ";
            int mainChoice; if (!(cin >> mainChoice)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
            string rest; getline(cin, rest);
            if (mainChoice == 0) break;

            if (mainChoice == 1) {
                bool adminExists = users.adminExists();
                cout << "Register as:\n";
                if (!adminExists) cout << "1) Admin\n2) Student\n";
                else cout << "1) Student\n";
                cout << "Choice: ";
                int rc; if (!(cin >> rc)) { cin.clear(); string tmp; getline(cin, tmp); continue; }
                getline(cin, rest);
                if (!adminExists) {
                    if (rc == 1) {
                        Passport p;
                        cout << "Enter admin info:\n";
                        p.inputFromConsole();
                        cout << "Username: "; string un; getline(cin, un);
                        cout << "Password: "; string pw; getline(cin, pw);
                        long id = users.createUser(un, hashPassword(pw), false, p);
                        cout << "Admin created id=" << id << "\n";
                    }
                    else if (rc == 2) {
                        Passport p;
                        cout << "Enter student info:\n";
                        p.inputFromConsole();
                        cout << "Username: "; string un; getline(cin, un);
                        cout << "Password: "; string pw; getline(cin, pw);
                        long id = users.createUser(un, hashPassword(pw), true, p);
                        cout << "Student created id=" << id << "\n";
                    }
                    else cout << "Invalid choice\n";
                }
                else {
                    if (rc == 1) {
                        Passport p;
                        cout << "Enter student info:\n";
                        p.inputFromConsole();
                        cout << "Username: "; string un; getline(cin, un);
                        cout << "Password: "; string pw; getline(cin, pw);
                        long id = users.createUser(un, hashPassword(pw), true, p);
                        cout << "Student created id=" << id << "\n";
                    }
                    else cout << "Invalid choice\n";
                }
            }
            else if (mainChoice == 2) {
                cout << "Login\nUsername: "; string un; getline(cin, un);
                cout << "Password: "; string pw; getline(cin, pw);
                bool isStudent = false;
                long id = users.checkLogin(un, hashPassword(pw), false, isStudent);
                if (id == -1) {
                    cout << "Invalid credentials\n";
                    continue;
                }
                if (!isStudent) {
                    cout << "Logged in as admin: " << un << "\n";
                    AdminUI adminui(users, testsdb, id);
                    adminui.run();
                }
                else {
                    cout << "Logged in as student: " << un << "\n";
                    StudentUI stui(users, testsdb, id, un);
                    stui.run();
                }
            }
            else {
                cout << "Invalid choice\n";
            }
        }

        users.close();
        testsdb.close();
    }
    catch (const exception& ex) {
        cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    cout << "Bye!\n";
    return 0;
}
