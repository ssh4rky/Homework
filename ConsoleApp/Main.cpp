#include <iostream>
#include <string>
#include <fstream>
#include <cctype>

using namespace std;

// 1.1
int* allocateArray(int size) {
    return new int[size];
}

void initializeArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        cout << "arr[" << i << "] = ";
        cin >> arr[i];
    }
}

void printArray(const int* arr, int size) {
    for (int i = 0; i < size; i++)
        cout << arr[i] << " ";
    cout << endl;
}

void deleteArray(int* arr) {
    delete[] arr;
}

void pushBack(int*& arr, int& size, int value) {
    int* temp = new int[size + 1];
    for (int i = 0; i < size; i++)
        temp[i] = arr[i];
    temp[size] = value;
    delete[] arr;
    arr = temp;
    size++;
}

void insertAt(int*& arr, int& size, int index, int value) {
    if (index < 0 || index > size) {
        cout << "Невірний індекс!" << endl;
        return;
    }
    int* temp = new int[size + 1];
    for (int i = 0; i < index; i++)
        temp[i] = arr[i];
    temp[index] = value;
    for (int i = index; i < size; i++)
        temp[i + 1] = arr[i];
    delete[] arr;
    arr = temp;
    size++;
}

void removeAt(int*& arr, int& size, int index) {
    if (index < 0 || index >= size) {
        cout << "Невірний індекс!" << endl;
        return;
    }
    int* temp = new int[size - 1];
    for (int i = 0; i < index; i++)
        temp[i] = arr[i];
    for (int i = index + 1; i < size; i++)
        temp[i - 1] = arr[i];
    delete[] arr;
    arr = temp;
    size--;
}

// 1.2
bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0) return false;
    return true;
}

int* removePrimes(const int* arr, int size, int& newSize) {
    newSize = 0;
    for (int i = 0; i < size; i++)
        if (!isPrime(arr[i]))
            newSize++;

    int* result = new int[newSize];
    int j = 0;
    for (int i = 0; i < size; i++)
        if (!isPrime(arr[i]))
            result[j++] = arr[i];

    return result;
}

// 1.3
void splitArray(const int* arr, int size,
    int*& positives, int& posSize,
    int*& negatives, int& negSize,
    int*& zeros, int& zeroSize)
{
    posSize = negSize = zeroSize = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) posSize++;
        else if (arr[i] < 0) negSize++;
        else zeroSize++;
    }

    positives = new int[posSize];
    negatives = new int[negSize];
    zeros = new int[zeroSize];

    int p = 0, n = 0, z = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) positives[p++] = arr[i];
        else if (arr[i] < 0) negatives[n++] = arr[i];
        else zeros[z++] = arr[i];
    }
}

// 2.1
void countSymbols(const char* str, int& letters, int& digits, int& others) {
    letters = digits = others = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
            letters++;
        else if (str[i] >= '0' && str[i] <= '9')
            digits++;
        else
            others++;
    }
}

// 2.2
int mystrcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    if (*str1 == *str2) return 0;
    return (*str1 > *str2) ? 1 : -1;
}

int StringToNumber(char* str) {
    int number = 0;
    bool isNegative = false;
    int i = 0;

    if (str[0] == '-') {
        isNegative = true;
        i++;
    }

    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            number = number * 10 + (str[i] - '0');
        }
        else break;
    }

    return isNegative ? -number : number;
}

char* NumberToString(int number) {
    string s = to_string(number);
    char* res = new char[s.size() + 1];
    strcpy(res, s.c_str());
    return res;
}

char* Uppercase(char* str1) {
    for (int i = 0; str1[i] != '\0'; i++) {
        str1[i] = toupper(str1[i]);
    }
    return str1;
}

char* Lowercase(char* str1) {
    for (int i = 0; str1[i] != '\0'; i++) {
        str1[i] = tolower(str1[i]);
    }
    return str1;
}

char* mystrrev(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
    return str;
}

// 3.1
void addColumn(int**& arr, int rows, int& cols, int pos) {
    if (pos < 0 || pos > cols) {
        cout << "Невірна позиція!" << endl;
        return;
    }

    for (int i = 0; i < rows; i++) {
        int* temp = new int[cols + 1];
        for (int j = 0; j < pos; j++)
            temp[j] = arr[i][j];
        temp[pos] = 0;
        for (int j = pos; j < cols; j++)
            temp[j + 1] = arr[i][j];
        delete[] arr[i];
        arr[i] = temp;
    }
    cols++;
}

// 3.2
void removeColumn(int**& arr, int rows, int& cols, int pos) {
    if (pos < 0 || pos >= cols) {
        cout << "Невірний номер стовпця!" << endl;
        return;
    }

    for (int i = 0; i < rows; i++) {
        int* temp = new int[cols - 1];
        for (int j = 0; j < pos; j++)
            temp[j] = arr[i][j];
        for (int j = pos + 1; j < cols; j++)
            temp[j - 1] = arr[i][j];
        delete[] arr[i];
        arr[i] = temp;
    }
    cols--;
}

// 3.3
void shiftRows(int** arr, int rows, int cols, int k) {
    k %= rows;
    for (int t = 0; t < k; t++) {
        int* temp = arr[rows - 1];
        for (int i = rows - 1; i > 0; i--)
            arr[i] = arr[i - 1];
        arr[0] = temp;
    }
}

void shiftCols(int** arr, int rows, int cols, int k) {
    k %= cols;
    for (int t = 0; t < k; t++) {
        for (int i = 0; i < rows; i++) {
            int temp = arr[i][cols - 1];
            for (int j = cols - 1; j > 0; j--)
                arr[i][j] = arr[i][j - 1];
            arr[i][0] = temp;
        }
    }
}

// 3.4
int** transpose(int** arr, int rows, int cols) {
    int** result = new int* [cols];
    for (int i = 0; i < cols; i++)
        result[i] = new int[rows];

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[j][i] = arr[i][j];

    return result;
}

// 3.5
struct Contact {
    string name;
    string phone;
};

void searchByName(Contact* arr, int size, const string& name) {
    for (int i = 0; i < size; i++) {
        if (arr[i].name == name) {
            cout << "Знайдено: " << arr[i].phone << endl;
            return;
        }
    }
    cout << "Не знайдено!" << endl;
}

void searchByPhone(Contact* arr, int size, const string& phone) {
    for (int i = 0; i < size; i++) {
        if (arr[i].phone == phone) {
            cout << "Знайдено: " << arr[i].name << endl;
            return;
        }
    }
    cout << "Не знайдено!" << endl;
}

// 4.6
int** multiplyMatrices(int** A, int M, int N, int** B, int K) {
    int** C = new int* [M];
    for (int i = 0; i < M; i++)
        C[i] = new int[K];

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            C[i][j] = 0;
            for (int t = 0; t < N; t++) {
                C[i][j] += A[i][t] * B[t][j];
            }
        }
    }
    return C;
}

//4.7
int** convert1Dto2D(int* arr, int size, int rows, int cols) {
    if (rows * cols != size) {
        cout << "Неможливо перетворити: розміри не співпадають!" << endl;
        return nullptr;
    }

    int** matrix = new int* [rows];
    for (int i = 0; i < rows; i++)
        matrix[i] = new int[cols];

    int k = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            matrix[i][j] = arr[k++];

    return matrix;
}

//4.8
string findLongestSurname(string arr[], int size) {
    string longest = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i].length() > longest.length()) {
            longest = arr[i];
        }
    }
    return longest;
}

//5.1
char* toBinary(int number) {
    char* result = new char[33];
    result[32] = '\0';

    for (int i = 31; i >= 0; --i) {
        result[i] = (number % 2) + '0';
        number /= 2;
    }

    int start = 0;
    while (result[start] == '0' && start < 31) start++;

    char* trimmed = new char[33 - start];
    strcpy(trimmed, result + start);
    delete[] result;
    return trimmed;
}

char* toHex(int number) {
    char* result = new char[9];
    sprintf(result, "%X", number);
    return result;
}

int fromBinary(const char* binaryStr) {
    return (int)strtol(binaryStr, nullptr, 2);
}

int fromHex(const char* hexStr) {
    return (int)strtol(hexStr, nullptr, 16);
}

//6.1
void compareFiles(const char* file1, const char* file2) {
    ifstream f1(file1);
    ifstream f2(file2);

    if (!f1.is_open() || !f2.is_open()) {
        cout << "Помилка відкриття файлів!" << endl;
        return;
    }

    string line1, line2;
    int lineNumber = 1;
    bool allMatch = true;

    while (true) {
        bool read1 = (bool)getline(f1, line1);
        bool read2 = (bool)getline(f2, line2);

        if (!read1 && !read2) break;
        if (line1 != line2) {
            allMatch = false;
            cout << "Рядок " << lineNumber << " відрізняється:" << endl;
            cout << "  Файл1: " << line1 << endl;
            cout << "  Файл2: " << line2 << endl;
        }
        lineNumber++;
    }

    if (allMatch) cout << "Файли повністю збігаються!" << endl;

    f1.close();
    f2.close();
}

//6.2
bool isVowel(char c) {
    c = tolower((unsigned char)c);
    return c == 'a' || c == 'e' || c == 'i' ||
        c == 'o' || c == 'u' || c == 'y';
}

void fileStatistics(const char* inputFile, const char* outputFile) {
    ifstream fin(inputFile);
    ofstream fout(outputFile);

    if (!fin.is_open() || !fout.is_open()) {
        cout << "Помилка відкриття файлів!" << endl;
        return;
    }

    long long charCount = 0, lineCount = 0, vowels = 0, consonants = 0, digits = 0;
    string line;

    while (getline(fin, line)) {
        lineCount++;
        for (char c : line) {
            charCount++;
            if (isdigit((unsigned char)c)) digits++;
            else if (isalpha((unsigned char)c)) {
                if (isVowel(c)) vowels++;
                else consonants++;
            }
        }
    }

    fout << "Кількість символів: " << charCount << endl;
    fout << "Кількість рядків: " << lineCount << endl;
    fout << "Кількість голосних: " << vowels << endl;
    fout << "Кількість приголосних: " << consonants << endl;
    fout << "Кількість цифр: " << digits << endl;

    cout << "Статистика записана у файл " << outputFile << endl;

    fin.close();
    fout.close();
}

int main() {
    
}

