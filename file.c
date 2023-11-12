#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#pragma pack(1)
struct BMPHeader
{
    // unsigned short signature;
    unsigned char signature[2];
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int dataOffset;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitCount;
    unsigned int compression;
    unsigned int imageSize;
    unsigned int xPixelsPerM;
    unsigned int yPixelsPerM;
    unsigned int colorsUsed;
    unsigned int colorsImportant;
};
#pragma pack()

void usage(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <fisier_intrare>\n", argv[0]);
        exit(1);
    }

    unsigned short int length = strlen(argv[1]);
    if (length < 4 || strcmp(argv[1] + length - 4, ".bmp") != 0)
    {
        printf("Error: File must have extenstion .bmp\n");
        exit(2);
    }
}

int main(int argc, char *argv[])
{
    usage(argc, argv);

    int f1;
    if ((f1 = open(argv[1], O_RDONLY)) < 0)
    {
        printf("Error opening input file\n");
        exit(3);
    }

    struct BMPHeader header;
    // printf("%ld\n", sizeof(header));
    if (read(f1, &header, sizeof(header)) != sizeof(header))
    {
        printf("Eroare la citirea header-ului\n");
        close(f1);
        exit(3);
    }
    // printf("%c %c\n", header.signature[0], header.signature[1]);
    // printf("%d\n", header.width);

    char output[1000];
    int output_size = sprintf(output, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\nidentificatorul utilizatorului: \ntimpul ultimei modificari: \ncontorul de legaturi: \ndrepturi de acces user: \ndrepturi de acces grup: \ndrepturi de acces altii: ",
                              argv[1], header.height, header.width, header.imageSize);

    int f2;
    if ((f2 = open("statistica.txt", O_WRONLY | O_CREAT | O_EXCL, S_IRWXU)) < 0)
    {
        printf("Error creating destination file\n");
        exit(3);
    }
    if (write(f2, output, output_size) < 0)
    {
        printf("Error writing to file\n");
        exit(6);
    }

    close(f1);
    close(f2);
    return 0;
}