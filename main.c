#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>

typedef struct MyFrame {
    int size_frame, flag1, flag2;
    char *name;
    char *content;
} MyFrame;

typedef struct MyFile {
    FILE *file;
    int size_frames, count;
    MyFrame *myFrame;
} MyFile;

void init_array(MyFile *myFile, int count) {
    myFile->myFrame = calloc(count, sizeof(MyFrame));
}

void resize_array(MyFile *myFile, int count) {
    myFile->myFrame = realloc(myFile->myFrame, sizeof(MyFrame) * count);
}

int get_size_tag(MyFile *myFile) {
    char header[3];
    for (int i = 0; i < 3; i++) {
        header[i] = (char) fgetc(myFile->file);
    }
    if (strncmp("ID3", header, strlen("ID3")) != 0) {
        return 0;
    }
    fseek(myFile->file, 3, SEEK_CUR);
    size_t byte0 = 128 * 128 * 128 * fgetc(myFile->file);
    size_t byte1 = 128 * 128 * fgetc(myFile->file);
    size_t byte2 = 128 * fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFile->size_frames = (int)(byte0 + byte1 + byte2 + byte3);
    return 1;
}

int parse_frame(MyFile *myFile, int counter) {
    MyFrame *myFrame = calloc(1, sizeof(MyFrame));
    myFrame->name = calloc(4, sizeof(char));
    for (int i = 0; i < 4; i++) {
        myFrame->name[i] = (char) fgetc(myFile->file);
        if (!(myFrame->name[i] <= 'Z' && myFrame->name[i] >= 'A' || myFrame->name[i] <= '9' && myFrame->name[i] >= '0')) {
            free(myFrame->name);
            free(myFrame);
            return 0;
        }
    }
    size_t byte0 = 128 * 128 * 128 * fgetc(myFile->file);
    size_t byte1 = 128 * 128 * fgetc(myFile->file);
    size_t byte2 = 128 * fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFrame->size_frame = (int)(byte0 + byte1 + byte2 + byte3);
    myFrame->flag1 = fgetc(myFile->file);
    myFrame->flag2 = fgetc(myFile->file);
    myFrame->content = (char *) calloc(myFrame->size_frame, sizeof(char));
    for (int i = 0; i < myFrame->size_frame; i++) {
        myFrame->content[i] = (char) fgetc(myFile->file);
    }
    if (strncmp("APIC", myFrame->name, strlen("APIC")) == 0) {
        return 0;
    }
    myFile->myFrame[counter] = *myFrame;
    return 1;
}

void parse_frames(MyFile *myFile) {
    int counter = 0, total = 4;
    init_array(myFile, total);
    while (ftell(myFile->file) < myFile->size_frames) {
        if (parse_frame(myFile, counter))
            counter++;
        if (counter == total) {
            total *= 2;
            resize_array(myFile, total);
            if (myFile->myFrame == NULL) {
                fprintf(stderr, "Not enough memory!");
                exit(1);
            }
        }
    }
    myFile->count = counter;
}

int main() {
    setlocale(LC_ALL, "Russian");
    FILE *file = fopen("../../3.mp3", "rb");
    if (file == NULL) {
        fprintf(stderr, "File does not exist!");
        return 1;
    }
    MyFile *myFile = calloc(1, sizeof(MyFile));
    myFile->file = file;
    if (get_size_tag(myFile) == 0) {
        fprintf(stderr, "FIle has an invalid structure!");
    }
    parse_frames(myFile);
    for (int i = 0; i < myFile->count; i++) {
        printf("%s", myFile->myFrame[i].name);
        if (myFile->myFrame->name == "APIC") continue;
        for (int j = 0; j < myFile->myFrame[i].size_frame; j++) {
            printf("%c", myFile->myFrame[i].content[j]);
        }
        printf("\n");
    }
    return 0;
}
