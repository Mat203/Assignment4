#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

typedef char *(__cdecl *encrypt)(char *, int);
typedef char *(__cdecl *decrypt)(char *, int);

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
    virtual void write(const string &text) = 0;
};

class FileWriter : public IWriter
{
private:
    string filePath;

public:
    FileWriter(string filePath) : filePath(filePath) {}
    void write(const string &text)
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

class CaesarCipher
{
private:
    typedef char *(__cdecl *encryptFunc)(char *, int);
    typedef char *(__cdecl *decryptFunc)(char *, int);

    HINSTANCE handle;
    encryptFunc encrypt_ptr;
    decryptFunc decrypt_ptr;

public:
    CaesarCipher(const char* dllPath)
    {
        handle = LoadLibrary(TEXT(dllPath));
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        {
            std::cout << "Lib not found" << std::endl;
            throw std::runtime_error("Lib not found");
        }

        encrypt_ptr = (encryptFunc)GetProcAddress(handle, TEXT("encrypt"));
        if (encrypt_ptr == nullptr)
        {
            std::cout << "Function not found" << std::endl;
            throw std::runtime_error("Function not found");
        }

        decrypt_ptr = (decryptFunc)GetProcAddress(handle, TEXT("decrypt"));
        if (decrypt_ptr == nullptr)
        {
            std::cout << "Function not found" << std::endl;
            throw std::runtime_error("Function not found");
        }
    }

    ~CaesarCipher()
    {
        FreeLibrary(handle);
    }

    string encryptString(const string& content, int key)
    {
        char* encryptedContent = (*encrypt_ptr)(const_cast<char*>(content.c_str()), key);
        string result(encryptedContent);
        delete[] encryptedContent;
        return result;
    }

    string decryptString(const string& content, int key)
    {
        char* decryptedContent = (*decrypt_ptr)(const_cast<char*>(content.c_str()), key);
        string result(decryptedContent);
        delete[] decryptedContent;
        return result;
    }
};


int main()
{
    CaesarCipher cipher("library.dll");

    srand(time(0));

    cout << "Choose mode (1 - Normal, 2 - Secret): ";
    int mode;
    cin >> mode;
    cin.ignore();
    cout << "Enter input file path: ";
    string inputPath;
    getline(cin, inputPath);
    cout << "Enter output file path: ";
    string outputPath;
    getline(cin, outputPath);
    string operation;
    int key;

    if (mode == 1)
    {
        cout << "Choose operation (encrypt or decrypt): ";
        getline(cin, operation);
        cout << "Enter the key: ";
        cin >> key;
        cin.ignore();
    }
    else
    {
        operation = "encrypt";
        key = rand();
        cout << "Generated key: " << key << endl;
    }
    
    IReader *reader = new FileReader(inputPath);
    
    string content = reader->read();
    
    if (!content.empty())
    {
        string processedContent;
        
        key = key % 128;

        if (operation == "encrypt")
            processedContent = cipher.encryptString(content, key);
        else
            processedContent = cipher.decryptString(content, key);

        
        IWriter *writer = new FileWriter(outputPath);
        
        writer->write(processedContent);

        
        delete writer;
        
     }
    
     delete reader;

     return 0;

}


