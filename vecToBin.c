#include<stdio.h>
#include<ctype.h>
#define MAX_STRING 100

int LINE_COUNTER(FILE *fp)
/* returns number of lines of fp and, set fp at the starting location */
{
    int count = 0;
 
    while(!feof(fp))
    {
        if(fgetc(fp) == '\n')
            count++;
    }
    rewind(fp);
 
    return count;
}

int DIMENTION_COUNTER(FILE *fp)
{
    int count = 0;
    char ch;
    int flag = 0;

    while((ch=fgetc(fp)) != '\n')
    {
        if(isspace(ch)&&flag==0)
        {
            count++;
            flag = 1;
        }
        else
            flag = 0;
    }
    rewind(fp);

    return count-1;
}

main(int argc, char *argv[])
{
    long long int vecCount, dim;
    int temp, i, j;
    FILE *vecFile, *binFile;
    char word[MAX_STRING];
    float feat;

    if (argc < 3) {
        printf("Usage: ./vec2bin <.vec-file> <.bin-file>\n");
        return 0;
    }


    vecFile = fopen(argv[1], "r");
    binFile = fopen(argv[2], "wb");

    dim = DIMENTION_COUNTER(vecFile);
    vecCount = LINE_COUNTER(vecFile);
    printf("%lld\n", dim);
    printf("%lld\n", vecCount);

    fprintf(binFile, "%lld %lld\n", vecCount, dim);

    for(i=0; i<vecCount; i++)
    {
        fscanf(vecFile, "%s", word);
        fprintf(binFile, "%s ", word);
        printf("%s\n", word);
        for(j=0; j<dim; j++)
        {
            fscanf(vecFile, "%f", &feat);
            fwrite(&feat, sizeof(float) , 1, binFile);
        }
        fprintf(binFile, "\n");
    }

    fclose(binFile);
    fclose(vecFile);

    printf("Conversion into binary format completed..\n");
    printf(".bin file saved at: %s\n", argv[2]);
}
