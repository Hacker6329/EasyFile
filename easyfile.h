#ifndef EASY_FILE_H
#define EASY_FILE_H

// Including Libraries
#include <stdio.h>
#include <stdlib.h>
#include "easystring.h"

// Defs
#define EASY_FILE_BUFFER_SIZE 1024
// Mode: Text
#define EASY_FILE_MODE_TEXT_READ_ONLY "r"
#define EASY_FILE_MODE_TEXT_WRITE_ONLY "w"
#define EASY_FILE_MODE_TEXT_APPEND "a"
#define EASY_FILE_MODE_TEXT_READ_WRITE "r+"
#define EASY_FILE_MODE_TEXT_NEW_FILE_READ_WRITE "w+"
#define EASY_FILE_MODE_TEXT_READ_APPEND "a+"

// Mode: Binary
#define EASY_FILE_MODE_BINARY_READ_ONLY "rb"
#define EASY_FILE_MODE_BINARY_WRITE_ONLY "wb"
#define EASY_FILE_MODE_BINARY_APPEND "ab"
#define EASY_FILE_MODE_BINARY_READ_WRITE "r+b"
#define EASY_FILE_MODE_BINARY_NEW_FILE_READ_WRITE "w+b"
#define EASY_FILE_MODE_BINARY_READ_APPEND "a+b"

// EasyFile Structure
typedef struct {
    FILE* filePointer;
    EasyString* accessMode;
    EasyString* filePath;
    int isBinaryFile;
}EasyFile;

// Functions Pre-Declaration: Universal
int file_check_integrity(const EasyFile* file);
int file_is_open(const EasyFile* file);
int file_exist(const EasyString* filePath);
int file_exist_c_str(const char* filePath);
int file_size(const EasyString* filePath);
void file_wipe(const EasyString* filePath);
void file_close(EasyFile** file);

// Functions Pre-Declaration: Text
EasyFile* file_text_open(const EasyString* filePath, const char* accessMode);
EasyFile* file_text_open_c_str(const char* filePath, const char* accessMode);
int file_text_read(const EasyFile* file, EasyString* buffer, int length);
int file_text_read_line(const EasyFile* file, EasyString* buffer);
int file_text_read_all(const EasyFile* file, EasyString* buffer);
int file_text_write(const EasyFile* file, const EasyString* buffer);

// Functions Pre-Declaration: Binary
EasyFile* file_binary_open(const EasyString* filePath, const char* accessMode);
EasyFile* file_binary_open_c_str(const char* filePath, const char* accessMode);
int file_binary_read(const EasyFile* file, void* buffer, int elementSize, int elementAmount);
int file_binary_read_all(const EasyFile* file, void* buffer);
int file_binary_write(const EasyFile* file, void* buffer, int elementSize, int elementAmount);

// Functions Declaration: Universal
int file_check_integrity(const EasyFile* file) {
    return (file == NULL || !string_check_integrity(file->accessMode) || !string_check_integrity(file->filePath) || (file->isBinaryFile>1 || file->isBinaryFile<0))==0;
}
int file_is_open(const EasyFile* file) {
    return (file_check_integrity(file) && file->filePointer!=NULL);
}
int file_exist(const EasyString* filePath) {
    if(!string_check_integrity(filePath)) return 0;
    return file_exist_c_str(filePath->string);
}
int file_exist_c_str(const char* filePath) {
    if(filePath == NULL) return 0;
    FILE* fp = fopen(filePath, EASY_FILE_MODE_TEXT_READ_ONLY);
    int result = fp != NULL;
    fclose(fp);
    return result;
}
int file_size(const EasyString* filePath) {
    if(!file_exist(filePath)) return -1;
    FILE* filePointer = fopen(string_c_str(filePath), EASY_FILE_MODE_TEXT_READ_ONLY);
    fseek(filePointer, 0L, SEEK_END);
    int fileSize = ftell(filePointer);
    fclose(filePointer);
    return fileSize;
}
void file_wipe(const EasyString* filePath) {
    if(!string_check_integrity(filePath)) return;
    fclose(fopen(string_c_str(filePath), EASY_FILE_MODE_TEXT_WRITE_ONLY));
}
void file_close(EasyFile** file) {
    if(file == NULL) return;
    if(!file_is_open(*file)) return;
    string_delete(&((*file)->accessMode));
    string_delete(&((*file)->filePath));
    fclose((*file)->filePointer);
    (*file)->filePointer = NULL;
    (*file)->isBinaryFile = -1;
    *file = NULL;
}

// Functions Declaration: Text
EasyFile* file_text_open(const EasyString* filePath, const char* accessMode) {
    if(!string_check_integrity(filePath) || accessMode == NULL) return NULL;
    return file_text_open_c_str(filePath->string, accessMode);
}
EasyFile* file_text_open_c_str(const char* filePath, const char* accessMode) {
    if(filePath == NULL || accessMode == NULL) return NULL;

    // Istanciate a new accessMode var and lower it to fit constants, returning in case an error has occurred
    EasyString* usedAccessMode = string_init_with_string(accessMode);
    if(usedAccessMode == NULL) return NULL;
    if(string_to_lower(usedAccessMode) == -1) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Prevent Invalid AccessMode
    if(
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_READ_ONLY) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_WRITE_ONLY) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_APPEND) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_READ_WRITE) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_NEW_FILE_READ_WRITE) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_TEXT_READ_APPEND)
    ) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Prevent Reading on unexisting file
    if(string_contains_c_str(string_c_str(usedAccessMode), "r") && !file_exist_c_str(filePath)) return NULL;

    // Open the C file
    FILE* filePointer = fopen(filePath, string_c_str(usedAccessMode));

    // Return if error occurred, noticed with NULL-check
    if(filePointer == NULL) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Allocate space for the structure & filling fields
    EasyFile* file = (EasyFile*) malloc(sizeof(EasyFile));
    file->isBinaryFile = 0;
    file->accessMode = string_init_with_string(string_c_str(usedAccessMode));
    file->filePath = string_init_with_string(filePath);
    file->filePointer = filePointer;

    return file;
}
int file_text_read(const EasyFile* file, EasyString* buffer, int length) {
    if(length <= 0 || !string_check_integrity(buffer) || !file_is_open(file) || file->isBinaryFile) return -1;

    char* fileBuffer = (char*) malloc(sizeof(char)*(length+1));

    int byteRead = fread(fileBuffer, sizeof(char), length, file->filePointer);

    if(byteRead > 0) {
        fileBuffer[byteRead] = 0;
        string_append_c_str(buffer, fileBuffer);
    }

    free(fileBuffer);

    return byteRead;
}
int file_text_read_line(const EasyFile* file, EasyString* buffer) {
    if(!string_check_integrity(buffer) || !file_is_open(file) || file->isBinaryFile) return -1;

    int charRead = 0;
    char fileBuffer[2];
    fileBuffer[1] = 0;

    do {
        fread(fileBuffer, sizeof(char), 1, file->filePointer);
        string_append_c_str(buffer, fileBuffer);
        charRead++;
    }while(fileBuffer[0] != '\n' && !feof(file->filePointer));

    if(charRead>0) charRead--;

    return charRead;

}
int file_text_read_all(const EasyFile* file, EasyString* buffer) {
    if(!string_check_integrity(buffer) || !file_is_open(file) || file->isBinaryFile) return -1;

    int totCharRead = 0, charRead;
    char fileBuffer[EASY_FILE_BUFFER_SIZE+1];

    while(!feof(file->filePointer)) {
        charRead = fread(fileBuffer, sizeof(char), EASY_FILE_BUFFER_SIZE, file->filePointer);
        fileBuffer[charRead] = 0;
        string_append_c_str(buffer, fileBuffer);
        totCharRead+=charRead;
    }

    return totCharRead;
}
int file_text_write(const EasyFile* file, const EasyString* buffer) {
    if(!string_check_integrity(buffer) || !file_is_open(file) || file->isBinaryFile) return -1;
    return fwrite(string_c_str(buffer), sizeof(char), string_length(buffer), file->filePointer);
}

// Functions Declaration: Binary
EasyFile* file_binary_open(const EasyString* filePath, const char* accessMode) {
    if(!string_check_integrity(filePath)) return NULL;
    return file_binary_open_c_str(filePath->string, accessMode);
}
EasyFile* file_binary_open_c_str(const char* filePath, const char* accessMode) {
    if(filePath == NULL || accessMode == NULL) return NULL;

    // Istanciate a new accessMode var and lower it to fit constants, returning in case an error has occurred
    EasyString* usedAccessMode = string_init_with_string(accessMode);
    if(usedAccessMode == NULL) return NULL;
    if(string_to_lower(usedAccessMode) == -1) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Prevent Invalid AccessMode
    if(
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_READ_ONLY) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_WRITE_ONLY) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_APPEND) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_READ_WRITE) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_NEW_FILE_READ_WRITE) &&
    !string_equals_c_str(string_c_str(usedAccessMode), EASY_FILE_MODE_BINARY_READ_APPEND)
    ) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Prevent Reading on unexisting file
    if(string_contains_c_str(string_c_str(usedAccessMode), "r") && !file_exist_c_str(filePath)) return NULL;

    // Open the C file
    FILE* filePointer = fopen(filePath, string_c_str(usedAccessMode));

    // Return if error occurred, noticed with NULL-check
    if(filePointer == NULL) {
        string_delete(&usedAccessMode);
        return NULL;
    }

    // Allocate space for the structure & filling fields
    EasyFile* file = (EasyFile*) malloc(sizeof(EasyFile));
    file->isBinaryFile = 1;
    file->accessMode = string_init_with_string(string_c_str(usedAccessMode));
    file->filePath = string_init_with_string(filePath);
    file->filePointer = filePointer;

    return file;
}
int file_binary_read(const EasyFile* file, void* buffer, int elementSize, int elementAmount) {
    if(buffer==NULL || !file_is_open(file) || !(file->isBinaryFile) || elementSize<=0 || elementAmount<=0) return -1;
    return fread(buffer, elementSize, elementAmount, file->filePointer);
}
int file_binary_write(const EasyFile* file, void* buffer, int elementSize, int elementAmount) {
    if(buffer==NULL || !file_is_open(file) || !(file->isBinaryFile) || elementSize<=0 || elementAmount<=0) return -1;
    return fwrite(buffer, elementSize, elementAmount, file->filePointer);
}

#endif