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

    friend ostream& operator<<(ostream& os, const String& str) {
        os << (str.mystr ? str.mystr : "");
        return os;
    }

    ~String() {
        delete[] mystr;
    }
};

int main() {
    String s1, s2;
    s1.SetString("Hello, world!");
    s2.SetString("Lol, world!");
    String s3;
    s3 = s2;

    cout << "s1: " << s1 << endl;
    cout << "s2: " << s2 << endl;
    cout << "s3: " << s3 << endl;

    cout << s1[1] << endl;

    cout << "Length of s2: " << s2.StringLength() << endl;
    cout << "s1 + s2: " << s1 + s2 << endl;
}

