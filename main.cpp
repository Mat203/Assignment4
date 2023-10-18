#include <iostream>
#include <fstream>
#include <string>

using namespace std;

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
    IReader* reader = new FileReader("test.txt");
    cout << reader->read() << endl;
    delete reader;

    IWriter* writer = new FileWriter("test1234.txt");
    writer->write("Asasdasd");
    delete writer;

    return 0;
}
