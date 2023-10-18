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

int main()
{
    IReader* reader = new FileReader("test.txt");
    cout << reader->read() << endl;
    delete reader;
    return 0;
}
