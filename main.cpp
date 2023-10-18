#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

typedef char* (__cdecl *encrypt)(char*, int);
typedef char* (__cdecl *decrypt)(char*, int);

class IReader
{
public:
    virtual ~IReader() {}
    virtual string read() = 0;
};

class FileReader : public IReader
{
private:
    string filePath;
public:
    FileReader(string filePath) : filePath(filePath) {}
    string read()
    {
        ifstream file(filePath);
        if (!file)
        {
            cout << "File does not exist" << endl;
            return "";
        }
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        return content;
    }
};

class IWriter
{
public:
    virtual ~IWriter() {}
    virtual void write(const string& text) = 0;
};

class FileWriter : public IWriter
{
private:
    string filePath;
public:
    FileWriter(string filePath) : filePath(filePath) {}
    void write(const string& text)
    {
        ifstream file(filePath);
        if (file)
        {
            cout << "File already exists" << endl;
            return;
        }
        ofstream outFile(filePath);
        outFile << text;
    }
};

int main()
{
    HINSTANCE handle = LoadLibrary(TEXT("library.dll"));
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
    {
        std::cout << "Lib not found" << std::endl;
        return 1;
    }

    encrypt encrypt_ptr = (encrypt)GetProcAddress(handle, TEXT("encrypt"));
    if (encrypt_ptr == nullptr)
    {
        std::cout << "Function not found" << std::endl;
        return 1;
    }

    decrypt decrypt_ptr = (decrypt)GetProcAddress(handle, TEXT("decrypt"));
    if (decrypt_ptr == nullptr)
    {
        std::cout << "Function not found" << std::endl;
        return 1;
    }

    IReader* reader = new FileReader("test.txt");
    string content = reader->read();
    
    if (!content.empty())
    {
        char* encryptedContent = (*encrypt_ptr)(const_cast<char*>(content.c_str()), 5);
        cout << "Encrypted content: " << encryptedContent << endl;

        char* decryptedContent = (*decrypt_ptr)(encryptedContent, 5);
        cout << "Decrypted content: " << decryptedContent << endl;

        delete[] encryptedContent;
        delete[] decryptedContent;
    }

    delete reader;

    IWriter* writer = new FileWriter("test1234.txt");
    writer->write("asdasd");
    
    delete writer;

    FreeLibrary(handle);

    return 0;
}
