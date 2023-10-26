#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <iostream>
#include <string>
#include <stack>
#include <sstream>

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
    ofstream outFile;

public:
    FileWriter(string filePath) : filePath(filePath), outFile(filePath)
    {
        if (!outFile)
        {
            cout << "File can't be opened" << endl;
        }
    }

    ~FileWriter()
    {
        if (outFile.is_open())
        {
            outFile.close();
        }
    }

    void write(const string &text)
    {
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
    CaesarCipher(const char *dllPath)
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

    string encryptString(const string &content, int key)
    {
        char *encryptedContent = (*encrypt_ptr)(const_cast<char *>(content.c_str()), key);
        string result(encryptedContent);
        delete[] encryptedContent;
        return result;
    }

    string decryptString(const string &content, int key)
    {
        char *decryptedContent = (*decrypt_ptr)(const_cast<char *>(content.c_str()), key);
        string result(decryptedContent);
        delete[] decryptedContent;
        return result;
    }
};

class Cursor
{
public:
    int line = -1;
    int index = 0;

    void move(int new_line, int new_index)
    {
        line = new_line;
        index = new_index;
    }
};

class Action
{
public:
    std::string command;
    std::string text;
    int line;
    int index;

    Action(const std::string &command, const std::string &text, int line, int index)
        : command(command), text(text), line(line), index(index) {}
};

class ActionStack
{
private:
    std::deque<Action> actions;

public:
    void push(const std::string &command, const std::string &text, int line, int index)
    {
        if (actions.size() == 3)
        {
            actions.pop_front();
        }
        actions.push_back(Action(command, text, line, index));
    }

    Action pop()
    {
        Action back = actions.back();
        actions.pop_back();
        return back;
    }

    const std::deque<Action> &get_actions() const
    {
        return actions;
    }

    bool empty()
    {
        return actions.empty();
    }
};

class Node
{
public:
    std::string value;
    Node *next;
};

class LinkedList
{
private:
    ActionStack undoStack;
    ActionStack redoStack;

public:
    Node *head;
    std::string clipboard;
    Cursor cursor;

    LinkedList()
    {
        head = nullptr;
    }

    int get_current_line()
    {
        int current_line = 0;
        Node *current = head;
        while (current->next != nullptr)
        {
            current = current->next;
            current_line++;
        }
        return current_line;
    }

    void new_line()
    {
        Node *new_node = new Node();
        new_node->next = nullptr;

        if (head == nullptr)
        {
            head = new_node;
        }
        else
        {
            Node *current = head;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = new_node;
        }
        int current_line = get_current_line();

        undoStack.push("new_line", "", current_line, 0);
        cursor.line += 1;
        cursor.index = 0;
    }

    void append_text(const std::string &text)
    {
        if (head == nullptr)
        {
            new_line();
        }

        Node *current = head;
        while (current->next != nullptr)
        {
            current = current->next;
        }

        int current_line = get_current_line();

        undoStack.push("insert", text, current_line, current->value.length());
        current->value += text;
        cursor.index += text.length();
    }

    void save_text_to_file(const std::string &filename)
    {
        FileWriter writer(filename);

        Node *current = head;
        while (current != NULL)
        {
            writer.write(current->value + "\n");
            current = current->next;
        }
    }

    void insert_text(int line, int index, const std::string &text)
    {
        if (line < 0 || index < 0)
        {
            printf("Invalid line or index\n");
            return;
        }

        Node *current = head;
        int current_line = 0;

        while (current != nullptr && current_line < line)
        {
            current = current->next;
            current_line++;
        }

        if (current == nullptr)
        {
            printf("Line not found\n");
            return;
        }

        int length = current->value.length();

        if (index > length)
        {
            printf("Index out of range\n");
            return;
        }

        std::string new_value = current->value.substr(0, index) + text + current->value.substr(index);

        current->value = new_value;
        cursor.index += text.length();
        undoStack.push("insert", text, line, index);
    }

    void search_text(const std::string &substr)
    {
        int line_number = 0;

        Node *current = head;
        while (current != nullptr)
        {
            size_t pos = 0;
            while ((pos = current->value.find(substr, pos)) != std::string::npos)
            {
                printf("Text is present in this position: Line %d, Index %zu\n", line_number, pos);
                pos += substr.length();
            }

            line_number++;
            current = current->next;
        }
    }

    void load_text_from_file(const std::string &filename)
    {
        FileReader reader(filename);
        std::istringstream contentStream(reader.read());

        if (contentStream.str().empty())
        {
            printf("Error opening file\n");
            return;
        }

        while (head != nullptr)
        {
            Node *current = head;
            head = head->next;
            delete current;
        }

        std::string line;
        while (std::getline(contentStream, line))
        {
            new_line();
            Node *current = head;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->value = line;
        }
    }

    void delete_text(int line, int index, int num)
    {
        if (line < 0 || index < 0)
        {
            printf("Invalid line or index\n");
            return;
        }

        Node *current = head;
        int current_line = 0;

        while (current != nullptr && current_line < line)
        {
            current = current->next;
            current_line++;
        }

        if (current == nullptr)
        {
            printf("Line not found\n");
            return;
        }

        int length = current->value.length();

        if (index + num > length)
        {
            printf("Index and number of symbols out of range\n");
            return;
        }

        std::string text = current->value.substr(index, num);
        undoStack.push("delete", text, line, index);

        std::string new_value = current->value.substr(0, index) + current->value.substr(index + num);

        current->value = new_value;
    }
    void copy_text(int line, int index, int num)
    {
        if (line < 0 || index < 0)
        {
            printf("Invalid line or index\n");
            return;
        }

        Node *current = head;
        int current_line = 0;

        while (current != nullptr && current_line < line)
        {
            current = current->next;
            current_line++;
        }

        if (current == nullptr)
        {
            printf("Line not found\n");
            return;
        }

        int length = current->value.length();

        if (index + num > length)
        {
            printf("Index and number of symbols out of range\n");
            return;
        }

        clipboard = current->value.substr(index, num);
    }

    void cut_text(int line, int index, int num)
    {
        copy_text(line, index, num);

        delete_text(line, index, num);
    }

    void insert_with_replacement(int line, int index, const std::string &text)
    {
        int num = text.length();
        Node *current = head;
        int current_line = 0;

        while (current != nullptr && current_line < line)
        {
            current = current->next;
            current_line++;
        }
        std::string old_text = current->value;
        delete_text(line, index, num);
        insert_text(line, index, text);
        if (!undoStack.empty())
            undoStack.pop();
        if (!undoStack.empty())
            undoStack.pop();
        undoStack.push("replace", old_text, line, 0);
    }

    void delete_line()
    {
        if (head == nullptr)
        {
            printf("No lines to delete\n");
            return;
        }

        Node *current = head;
        Node *previous = nullptr;

        while (current->next != nullptr)
        {
            previous = current;
            current = current->next;
            cursor.index = previous->value.length();
        }

        if (previous == nullptr)
        {
            head = nullptr;
        }
        else
        {
            previous->next = nullptr;
        }

        delete current;
    }

    void undo()
    {
        if (undoStack.empty())
        {
            printf("Nothing to undo\n");
            return;
        }

        Action action = undoStack.pop();

        if (action.command == "insert")
        {
            delete_text(action.line, action.index, action.text.length());
            undoStack.pop();
            cursor.index -= action.text.length();
            redoStack.push(action.command, action.text, action.line, action.index);
        }
        else if (action.command == "delete")
        {
            insert_text(action.line, action.index, action.text);
            undoStack.pop();
            redoStack.push(action.command, action.text, action.line, action.index);
        }
        if (action.command == "new_line")
        {
            delete_line();
            cursor.line -= 1;
            redoStack.push(action.command, action.text, action.line, action.index);
        }
        if (action.command == "replace")
        {
            Node *current = head;
            int current_line = 0;

            while (current != nullptr && current_line < action.line)
            {
                current = current->next;
                current_line++;
            }

            if (current == nullptr)
            {
                printf("Line not found\n");
                return;
            }

            redoStack.push(action.command, current->value, action.line, action.index);

            current->value = action.text;
        }
    }

    void redo()
    {
        if (redoStack.empty())
        {
            printf("Nothing to redo\n");
            return;
        }

        Action action = redoStack.pop();

        if (action.command == "insert")
        {
            insert_text(action.line, action.index, action.text);
            undoStack.push(action.command, action.text, action.line, action.index);
        }
        else if (action.command == "delete")
        {
            delete_text(action.line, action.index, action.text.length());
            cursor.index -= action.text.length();
            undoStack.push(action.command, action.text, action.line, action.index);
        }
        if (action.command == "new_line")
        {
            new_line();
            undoStack.push(action.command, action.text, action.line, action.index);
        }
        if (action.command == "replace")
        {
            Node *current = head;
            int current_line = 0;

            while (current != nullptr && current_line < action.line)
            {
                current = current->next;
                current_line++;
            }

            if (current == nullptr)
            {
                printf("Line not found\n");
                return;
            }
            undoStack.push(action.command, current->value, action.line, action.index);

            current->value = action.text;
        }
    }
};

int main()
{
    printf("You can use next commands: \n");
    printf("1 - Text append\n");
    printf("2 - New line start\n");
    printf("3 - Save the text to the file\n");
    printf("4 - Load the text from the file\n");
    printf("5 - Current text print\n");
    printf("6 - Insert the text by line and symbol index\n");
    printf("7 - Text search\n");
    printf("8 - Delete symbols\n");
    printf("9 - Undo command\n");
    printf("10 - Redo command\n");
    printf("11 - Cut symbols\n");
    printf("12 - Paste symbols\n");
    printf("13 - Copy symbols\n");
    printf("14 - Insert with replacement\n");
    printf("15 - Move the cursor\n");

    LinkedList list;

    while (true)
    {
        int command;
        printf("Write your command:");
        scanf_s("%d", &command);

        if (command == 1)
        {
            char text[80];
            printf("Enter text to append: ");
            scanf_s("%s", text, sizeof(text));
            list.append_text(text);
        }

        if (command == 2)
        {
            printf("Hew line has started\n");
            list.new_line();
        }

        if (command == 3)
        {
            char filename[80];
            printf("Enter the file name for saving: ");
            scanf_s("%s", filename, sizeof(filename));
            list.save_text_to_file(filename);
            printf("Text has been saved successfully\n");
        }

        if (command == 4)
        {
            char filename[80];
            printf("Enter the file name for loading: ");
            scanf_s("%s", filename, sizeof(filename));
            list.load_text_from_file(filename);
            printf("Text has been loaded successfully\n");
        }

        if (command == 5)
        {
            Node *current = list.head;

            while (current != nullptr)
            {
                printf("%s\n", current->value.c_str());
                current = current->next;
            }
        }

        if (command == 6)
        {
            char text[80];
            printf("Enter the text to insert: ");
            scanf_s("%s", text, sizeof(text));

            list.insert_text(list.cursor.line, list.cursor.index, text);
        }

        if (command == 7)
        {
            char substring[80];
            printf("Enter text to search: ");
            scanf_s("%s", substring, sizeof(substring));
            list.search_text(substring);
        }

        if (command == 8)
        {
            int num;
            printf("Enter the number of symbols: ");
            scanf_s("%d", &num);

            list.delete_text(list.cursor.line, list.cursor.index, num);
        }

        if (command == 9)
        {
            list.undo();
        }

        if (command == 10)
        {
            list.redo();
        }

        if (command == 11)
        {
            int num;
            printf("Enter the number of symbols: ");
            scanf_s("%d", &num);

            list.cut_text(list.cursor.line, list.cursor.index, num);
        }

        if (command == 12)
        {
            list.insert_text(list.cursor.line, list.cursor.index, list.clipboard);
        }

        if (command == 13)
        {
            int line, index, num;
            printf("Enter the number of line, index and number of symbols: ");
            scanf_s("%d %d %d", &line, &index, &num);

            list.copy_text(line, index, num);
        }

        if (command == 14)
        {
            int line, index;
            char text[80];
            printf("Enter the text to insert: ");
            scanf_s("%s", text, sizeof(text));

            list.insert_with_replacement(list.cursor.line, list.cursor.index, text);
        }

        if (command == 15)
        {
            int line, index;
            printf("Enter the number of line and index: ");
            scanf_s("%d %d", &line, &index);

            list.cursor.move(line, index);
        }

        if (command == 16)
        {
            printf("Cursor line % d, index% d\n", list.cursor.line, list.cursor.index);
        }
    }

    return 0;
}

/*int main()
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
*/
