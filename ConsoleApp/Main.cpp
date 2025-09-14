#include <iostream>

using namespace std;

class String {
private:
    char* mystr;
public:
    String() : mystr(nullptr) {}

    String(const char* str) {
        if (str) {
            mystr = new char[strlen(str) + 1];
            strcpy_s(mystr, strlen(str) + 1, str);
        }
        else {
            mystr = nullptr;
        }
    }

    String(String&& other) noexcept {
        mystr = other.mystr;
        other.mystr = nullptr;
    }

    String(const String& other) {
        if (other.mystr) {
            mystr = new char[strlen(other.mystr) + 1];
            strcpy_s(mystr, strlen(other.mystr) + 1, other.mystr);
        }
        else {
            mystr = nullptr;
        }
    }

    String& operator=(const String& other) {
        if (this != &other) {
            delete[] mystr;
            if (other.mystr) {
                mystr = new char[strlen(other.mystr) + 1];
                strcpy_s(mystr, strlen(other.mystr) + 1, other.mystr);
            }
            else {
                mystr = nullptr;
            }
        }
        return *this;
    }

    String& operator=(String&& other) noexcept {
        if (this != &other) {
            delete[] mystr;

            mystr = other.mystr;
            other.mystr = nullptr;
        }
        return *this;
    }


    String operator+(const String& other) const {
        int len1 = mystr ? strlen(mystr) : 0;
        int len2 = other.mystr ? strlen(other.mystr) : 0;

        char* newstr = new char[len1 + len2 + 1];

        if (mystr)
            strcpy_s(newstr, len1 + len2 + 1, mystr);
        else
            newstr[0] = '\0';

        if (other.mystr)
            strcat_s(newstr, len1 + len2 + 1, other.mystr);

        String result(newstr);
        delete[] newstr;
        return result;
    }

    char& operator[](int value) {
        return mystr[value];
    }

    const char& operator[](int value) const {
        return mystr[value];
    }

    bool operator==(const String& other) const {
        if (this->mystr == nullptr && other.mystr == nullptr) {
            return true;
        }
        if (this->mystr == nullptr || other.mystr == nullptr) {
            return false;
        }
        return strcmp(this->mystr, other.mystr);
    }

    bool operator!=(const String& other) const {
        return !(this->mystr == other.mystr);
    }


    void SetString(const char* str) {
        delete[] mystr;
        if (str) {
            mystr = new char[strlen(str) + 1];
            strcpy_s(mystr, strlen(str) + 1, str);
        }
        else {
            mystr = nullptr;
        }
    }

    int StringLength() const {
        return mystr ? strlen(mystr) : 0;
    }

    const char* PrintStr() const {
        return mystr ? mystr : "";
    }

    void Erase(int index, int amount) {
        if (!mystr) {
            cerr << "This string is empty" << endl;
            return;
        }

        int len = StringLength();

        if (index < 0 || index >= len) {
            throw out_of_range("Index out of range");
        }

        if (amount <= 0) return;

        if (index + amount > len) {
            amount = len - index;
        }

        for (int i = index; i <= len - amount; i++) {
            mystr[i] = mystr[i + amount];
        }
    }

    int Find(const String& substr) const {
        if (!mystr || !substr.mystr) return -1;

        int len = StringLength();
        int sublen = substr.StringLength();

        if (sublen == 0) return 0;

        for (int i = 0; i <= len - sublen; i++) {
            bool match = true;
            for (int j = 0; j < sublen; j++) {
                if (mystr[i + j] != substr.mystr[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }

        return -1;
    }

    friend ostream& operator<<(ostream& os, const String& str) {
        os << (str.mystr ? str.mystr : "");
        return os;
    }

    ~String() {
        delete[] mystr;
    }
};

int main() {
    String s1, s2, s3;
    s1.SetString("Hello, world!");
    s2.SetString("Lol, world!");
    s3.SetString("world!");

    cout << "s1: " << s1 << endl;
    cout << "s2: " << s2 << endl;
    cout << "s3: " << s3 << endl;

    cout << s2.Find(s3) << endl;
    cout << s1[1] << endl;

    cout << "Length of s2: " << s2.StringLength() << endl;
    cout << "s1 + s2: " << s1 + s2 << endl;
}

