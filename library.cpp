#include <iostream>
#include <cstring>
#include <sstream>

extern "C" {
    __declspec(dllexport) char* encrypt(char* rawText, int key) {
        key = key % 128; 

        int length = strlen(rawText);
        char* encryptedText = new char[length + 1];

        for (int i = 0; i < length; i++) {
            char c = rawText[i];
            c = c + key;
            encryptedText[i] = c;
        }

        encryptedText[length] = '\0';
        return encryptedText;
    }

    __declspec(dllexport) char* decrypt(char* encryptedText, int key) {
        key = key % 128; 

        int length = strlen(encryptedText);
        char* decryptedText = new char[length + 1];

        for (int i = 0; i < length; i++) {
            char c = encryptedText[i];
            c = c - key;
            decryptedText[i] = c;
        }

        decryptedText[length] = '\0';
        return decryptedText;
    }
}
