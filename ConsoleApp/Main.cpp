#include <iostream>

using namespace std;

//Two-sided LinkedList
template <typename T>
struct Node {
    T value;
    Node<T>* previous;
    Node<T>* next;
    Node(T val) : value(val), previous(nullptr), next(nullptr) {}
};

template<typename T>
class LinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int size;
public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}
    ~LinkedList();

    void Append(T val);
    void InsertAt(int index, T val);
    void PrintForward() const;
    void PrintBackward() const;
    T RemoveAt(int index);

    int GetSize() const { return size; }
    Node<T>* GetHead() const { return head; }
    Node<T>* GetTail() const { return tail; }
};

template <typename T>
void LinkedList<T>::Append(T val) {
    Node<T>* newNode = new Node<T>(val);
    if (tail == nullptr) {
        head = tail = newNode;
    }
    else {
        tail->next = newNode;
        newNode->previous = tail;
        tail = newNode;
    }
    size++;
}

template<typename T>
LinkedList<T>::~LinkedList() {
    Node<T>* current = head;
    while (current) {
        Node<T>* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}

template<typename T>
T LinkedList<T>::RemoveAt(int index) {
    if (index < 0 || index >= size) {
        throw out_of_range("Index out of Bounds");
    }

    Node<T>* toDelete = head;

    if (index == 0) {
        head = head->next;
        if (head) head->previous = nullptr;
        if (toDelete == tail) tail = nullptr;
    }
    else {
        for (int i = 0; i < index; i++) {
            toDelete = toDelete->next;
        }
        Node<T>* prevNode = toDelete->previous;
        Node<T>* nextNode = toDelete->next;

        if (prevNode) prevNode->next = nextNode;
        if (nextNode) nextNode->previous = prevNode;
        if (toDelete == tail) tail = prevNode;
    }
    size--;
    return toDelete->value;
    delete toDelete;
}

template <typename T>
void LinkedList<T>::PrintForward() const {
    Node<T>* current = head;
    while (current != nullptr) {
        cout << current->value << " ";
        current = current->next;
    }
    cout << endl;
}

template <typename T>
void LinkedList<T>::PrintBackward() const {
    Node<T>* current = tail;
    while (current != nullptr) {
        cout << current->value << " ";
        current = current->previous;
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
    LinkedList<int> list;
    list.Append(10);
    list.Append(12);
    list.Append(14);
    list.Append(16);

    cout << "Forward: ";
    list.PrintForward();

    cout << "Backward: ";
    list.PrintBackward();

    list.RemoveAt(2);

    cout << "After removal: ";
    list.PrintForward();
}

