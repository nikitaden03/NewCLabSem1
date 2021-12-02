#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct MyFrame {
    int size_frame, flag1, flag2;
    char *name;
    char *content;
} MyFrame;

FILE *temp_file;

typedef struct MyFile {
    FILE *file;
    int size_frames, count, difference;
    char *path;
    MyFrame *myFrame;
} MyFile;

void init_array(MyFile *myFile, int count) {
    myFile->myFrame = calloc(count, sizeof(MyFrame));
}

void resize_array(MyFile *myFile, int count) {
    myFile->myFrame = realloc(myFile->myFrame, sizeof(MyFrame) * count);
}

void init_file(char *str, MyFile *myFile) {
    FILE *file = fopen(str, "rb");
    myFile->path = str;
    if (file == NULL) {
        fprintf(stderr, "File does not exist!");
        exit(1);
    }
    myFile->file = file;
}

int get_size_tag(MyFile *myFile, int save_mode) {
    char header[3];
    for (int i = 0; i < 3; i++) {
        header[i] = (char) fgetc(myFile->file);
    }
    if (strncmp("ID3", header, strlen("ID3")) != 0) {
        return 0;
    }
    char c1 = (char) fgetc(myFile->file);
    char c2 = (char) fgetc(myFile->file);
    char c3 = (char) fgetc(myFile->file);
    size_t byte0 = fgetc(myFile->file);
    size_t byte1 = fgetc(myFile->file);
    size_t byte2 = fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFile->size_frames = (int) (128 * 128 * 128 * byte0 + 128 * 128 * byte1 + 128 * byte2 + byte3);
    if (save_mode) {
        fprintf(temp_file, "%c%c%c%c%c%c%c%c%c%c", header[0], header[1], header[2], c1, c2, c3, byte0, byte1, byte2,
                byte3);
    }
    return 1;
}

int parse_frame(MyFile *myFile, int counter, int save_mode, const char *tag_name, const char *tag_value) {
    MyFrame *myFrame = calloc(1, sizeof(MyFrame));
    myFrame->name = calloc(4, sizeof(char));
    for (int i = 0; i < 4; i++) {
        myFrame->name[i] = (char) fgetc(myFile->file);
        if (save_mode) {
            fprintf(temp_file, "%c", myFrame->name[i]);
        }
        if (!(myFrame->name[i] <= 'Z' && myFrame->name[i] >= 'A' ||
              myFrame->name[i] <= '9' && myFrame->name[i] >= '0')) {
            free(myFrame->name);
            free(myFrame);
            return 0;
        }
    }
    size_t byte0 = fgetc(myFile->file);
    size_t byte1 = fgetc(myFile->file);
    size_t byte2 = fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFrame->size_frame = (int) (128 * 128 * 128 * byte0 + 128 * 128 * byte1 + 128 * byte2 + byte3);
    myFrame->flag1 = fgetc(myFile->file);
    myFrame->flag2 = fgetc(myFile->file);
    myFrame->content = (char *) calloc(myFrame->size_frame, sizeof(char));
    for (int i = 0; i < myFrame->size_frame; i++) {
        myFrame->content[i] = (char) fgetc(myFile->file);
    }
    if (save_mode) {
        if (myFrame->name[0] == tag_name[0] && myFrame->name[1] == tag_name[1] &&
            myFrame->name[3] == tag_name[3] && myFrame->name[2] == tag_name[2] &&
            strlen(tag_name) == 4) {
            myFrame->content = realloc(myFrame->content, strlen(tag_value));
            myFile->difference = (int)(strlen(tag_name) - myFrame->size_frame);
            myFrame->size_frame = (int) strlen(tag_value);
            for (int j = 0; j < myFrame->size_frame; j++) {
                myFrame->content[j] = tag_value[j];
            }
            byte0 = myFrame->size_frame / (128 * 128 * 128);
            byte1 = myFrame->size_frame / (128 * 128) % 128;
            byte2 = myFrame->size_frame / 128 % 128;
            byte3 = myFrame->size_frame % 128;
        }
        fprintf(temp_file, "%c%c%c%c%c%c", byte0, byte1, byte2, byte3, myFrame->flag1, myFrame->flag2);
        for (int j = 0; j < myFrame->size_frame; j++) {
            fprintf(temp_file, "%c", myFrame->content[j]);
        }
    }
    if (strncmp("APIC", myFrame->name, strlen("APIC")) == 0) {
        return 0;
    }
    if (!save_mode)
        myFile->myFrame[counter] = *myFrame;
    return 1;
}

void parse_frames(MyFile *myFile, int save_mode, const char *tag_name, const char *tag_value) {
    int counter = 0, total = 4;
    init_array(myFile, total);
    while (ftell(myFile->file) < myFile->size_frames) {
        if (parse_frame(myFile, counter, save_mode, tag_name, tag_value) && !save_mode)
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
    if (save_mode) {
        char c;
        while (fscanf(myFile->file, "%c", &c) != EOF) {
            fprintf(temp_file, "%c", c);
        }
        fclose(temp_file);
        temp_file = fopen("../../temp_file.mp3", "rb+");
        int pointer = ftell(temp_file);
        fseek(temp_file, 6, SEEK_SET);
        myFile->size_frames += myFile->difference;
        int byte0 = myFile->size_frames / (128 * 128 * 128);
        int byte1 = myFile->size_frames  / (128 * 128) % 128;
        int byte2 = myFile->size_frames  / 128 % 128;
        int byte3 = myFile->size_frames  % 128;
        fprintf(temp_file, "%c%c%c%c", byte0, byte1, byte2, byte3);
        fseek(temp_file, pointer, SEEK_SET);
    }
    myFile->count = counter;
}

void show_tags(MyFile *myFile) {
    for (int i = 0; i < myFile->count; i++) {
        printf("%s -", myFile->myFrame[i].name);
        if (myFile->myFrame[i].content[0] != '\0') {
            printf(" ");
        }
        if (strcmp(myFile->myFrame->name, "APIC") == 0) continue;
        for (int j = 0; j < myFile->myFrame[i].size_frame; j++) {
            printf("%c", myFile->myFrame[i].content[j]);
        }
        printf("\n");
    }
}

void show_tag(MyFile *myFile, const char *tag_name) {
    for (int i = 0; i < myFile->count; i++) {
        if (myFile->myFrame[i].name[0] == tag_name[0] && myFile->myFrame[i].name[1] == tag_name[1] &&
            myFile->myFrame[i].name[3] == tag_name[3] && myFile->myFrame[i].name[2] == tag_name[2] &&
            strlen(tag_name) == 4) {
            printf("%s -", myFile->myFrame[i].name);
            if (myFile->myFrame[i].content[0] != '\0') {
                printf(" ");
            }
            for (int j = 0; j < myFile->myFrame[i].size_frame; j++) {
                printf("%c", myFile->myFrame[i].content[j]);
            }
            return;
        }
    }
    printf("Tag with this name does not exist!");
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");

    int c, help = 0;

    MyFile *myFile = calloc(1, sizeof(MyFile));

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"filepath", 1, 0, 0},
                {"show",     0, 0, 1},
                {"set",      1, 0, 2},
                {"value",    1, 0, 3},
                {"get",      1, 0, 4}
        };
        c = getopt_long(argc, argv, "1",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                init_file(optarg, myFile);
                break;
            case 1:
                if (get_size_tag(myFile, 0) == 0) {
                    fprintf(stderr, "FIle has an invalid structure!");
                    return 1;
                }
                parse_frames(myFile, 0, "", "");
                show_tags(myFile);
                break;
            case 2:
                help++;
                char *label = optarg;
                c = getopt_long(argc, argv, "1",
                                long_options, &option_index);
                if (c != 3)
                    break;
                char *value = optarg;
                temp_file = fopen("../../temp_file.mp3", "wb");
                if (get_size_tag(myFile, 1) == 0) {
                    fprintf(stderr, "FIle has an invalid structure!");
                    return 1;
                }
                parse_frames(myFile, 1, label, value);
                fclose(temp_file);
                fclose(myFile->file);
                char *true_path = malloc(strlen(myFile->path));
                for (int i = 0; i < strlen(myFile->path); i++) {
                    true_path[i] = myFile->path[i];
                }
                remove(true_path);
                rename("../../temp_file.mp3", true_path);
                break;
            case 4:
                if (get_size_tag(myFile, 0) == 0) {
                    fprintf(stderr, "FIle has an invalid structure!");
                    return 1;
                }
                parse_frames(myFile, 0, "", "");
                show_tag(myFile, optarg);
                break;
            default:
                break;
        }
    }
    fclose(myFile->file);
    return 0;
}
