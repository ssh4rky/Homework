#include <iostream>

using namespace std;

template <typename T>
struct Node {
    T value;
    Node<T>* prev;
    Node<T>* next;
    Node(T val) : value(val), prev(nullptr), next(nullptr) {}
};

template <typename T>
class DoublyLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int count;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}
    ~DoublyLinkedList();

    void push_front(T value);

    void push_back(T value);

    void pop_front();

    void pop_back();

    void insert(int position, T value);

    void erase(int position);

    Node<T>* find(T value);

    void clear();

    int size() const { return count; }

    bool empty() const { return count == 0; }

    void print_forward() const;

    void print_backward() const;
};

// Realizitaion
template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() {
    clear();
}

template <typename T>
void DoublyLinkedList<T>::push_front(T value) {
    Node<T>* newNode = new Node<T>(value);
    if (!head) {
        head = tail = newNode;
    }
    else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    count++;
}

template <typename T>
void DoublyLinkedList<T>::push_back(T value) {
    Node<T>* newNode = new Node<T>(value);
    if (!tail) {
        head = tail = newNode;
    }
    else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    count++;
}

template <typename T>
void DoublyLinkedList<T>::pop_front() {
    if (!head) return;
    Node<T>* temp = head;
    head = head->next;
    if (head) head->prev = nullptr;
    else tail = nullptr;
    delete temp;
    count--;
}

template <typename T>
void DoublyLinkedList<T>::pop_back() {
    if (!tail) return;
    Node<T>* temp = tail;
    tail = tail->prev;
    if (tail) tail->next = nullptr;
    else head = nullptr;
    delete temp;
    count--;
}

template <typename T>
void DoublyLinkedList<T>::insert(int position, T value) {
    if (position < 0 || position > count) throw out_of_range("Invalid position");

    if (position == 0) {
        push_front(value);
        return;
    }
    if (position == count) {
        push_back(value);
        return;
    }

    Node<T>* newNode = new Node<T>(value);
    Node<T>* current = head;
    for (int i = 0; i < position; i++) current = current->next;

    newNode->next = current;
    newNode->prev = current->prev;
    current->prev->next = newNode;
    current->prev = newNode;
    count++;
}

template <typename T>
void DoublyLinkedList<T>::erase(int position) {
    if (position < 0 || position >= count) throw out_of_range("Invalid position");

    if (position == 0) {
        pop_front();
        return;
    }
    if (position == count - 1) {
        pop_back();
        return;
    }

    Node<T>* current = head;
    for (int i = 0; i < position; i++) current = current->next;

    current->prev->next = current->next;
    current->next->prev = current->prev;
    delete current;
    count--;
}

template <typename T>
Node<T>* DoublyLinkedList<T>::find(T value) {
    Node<T>* current = head;
    while (current) {
        if (current->value == value) return current;
        current = current->next;
    }
    return nullptr;
}

template <typename T>
void DoublyLinkedList<T>::clear() {
    while (head) {
        pop_front();
    }
}

template <typename T>
void DoublyLinkedList<T>::print_forward() const {
    Node<T>* current = head;
    while (current) {
        cout << current->value << " ";
        current = current->next;
    }
    cout << endl;
}

template <typename T>
void DoublyLinkedList<T>::print_backward() const {
    Node<T>* current = tail;
    while (current) {
        cout << current->value << " ";
        current = current->prev;
    }
    cout << endl;
}

//String
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

    void replace(int index, int count, const String& replacement) {
        if (!mystr) return;

        int len = StringLength();
        if (index < 0 || index > len) {
            throw out_of_range("Index out of range");
        }

        if (count < 0) count = 0;
        if (index + count > len) {
            count = len - index;
        }

        int replLen = replacement.StringLength();
        int newLen = len - count + replLen;

        char* newStr = new char[newLen + 1];

        strncpy_s(newStr, newLen + 1, mystr, index);
        newStr[index] = '\0';

        strcat_s(newStr, newLen + 1, replacement.mystr);

        strcat_s(newStr, newLen + 1, mystr + index + count);

        delete[] mystr;
        mystr = newStr;
    }

    String substr(int start, int end) const {
        if (!mystr) return String("");
        int len = StringLength();

        if (start < 0 || start >= len || end < start) {
            throw out_of_range("Invalid substring range");
        }
        if (end >= len) end = len - 1;

        int subLen = end - start + 1;
        char* buffer = new char[subLen + 1];
        strncpy_s(buffer, subLen + 1, mystr + start, subLen);
        buffer[subLen] = '\0';

        String result(buffer);
        delete[] buffer;
        return result;
    }

    bool empty() const {
        return !mystr || mystr[0] == '\0';
    }

    void insert(int index, const String& toInsert) {
        if (!toInsert.mystr) return;
        if (!mystr) {
            SetString(toInsert.mystr);
            return;
        }

        int len = StringLength();
        if (index < 0 || index > len) {
            throw out_of_range("Index out of range");
        }

        int insLen = toInsert.StringLength();
        int newLen = len + insLen;

        char* newStr = new char[newLen + 1];

        strncpy_s(newStr, newLen + 1, mystr, index);
        newStr[index] = '\0';

        strcat_s(newStr, newLen + 1, toInsert.mystr);

        strcat_s(newStr, newLen + 1, mystr + index);

        delete[] mystr;
        mystr = newStr;
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
    //String
    String s("Hello World!");

    cout << "Original: " << s << endl;

    s.replace(6, 5, String("C++"));
    cout << "After replace: " << s << endl;

    String sub = s.substr(0, 4);
    cout << "Substring (0-4): " << sub << endl;

    cout << "Empty? " << (s.empty() ? " Yes" : " No") << endl;

    s.insert(6, String("amazing "));
    cout << "After insert: " << s << endl;

    //Two-sided LinkedList
    DoublyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_front(5);
    list.insert(1, 15);
    list.print_forward();
    list.print_backward();

    list.erase(2);
    list.print_forward();

    cout << "Find 15: " << (list.find(15) ? "Found" : "Not Found") << endl;
    cout << "Size: " << list.size() << endl;
    cout << "Empty: " << (list.empty() ? "Yes" : "No") << endl;

    list.clear();
    cout << "Empty after clear: " << (list.empty() ? "Yes" : "No") << endl;
}

