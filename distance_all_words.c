/*
*    Modified version of distance.c.
*    It creates a file named "filename.KNN" that contains all the words vocabulary along with 
*    their K nearest neighbours.
*    The default value of K is set to 30.
*    The first line of the output file will contain: <no-of-word-in-voca> <K>
*    Output:
*        <root-term_1>\t<nn_1:sim_1>\t<nn_2:sim_2>\t ... <nn_K:sim_K>\t\n
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <getopt.h>

const long long max_size = 2000;         // max length of strings
int N = 30;                  // number of closest words that will be shown
const long long max_w = 1024;              // max length of vocabulary entries

void print_usage() {
    printf("Usage: distance_query_words "
        "-i <FILE.bin: FILE contains word projections in the BINARY FORMAT> "
        "[-k <k to calculate kNN (default-30)>] "
        "[-o <output-file (default-stdout)>]\n");
}

int main(int argc, char **argv) {
    FILE *bin_file_fp;         // Input .bin file pointer
    char st1[max_size];
    char **bestw;
    float *bestd;
    char bin_file[max_size], st[100][max_size], output_filename[max_size];
    float dist, len, vec[max_size];
    long long words, size, a, b, c, d, cn, bi[100], i;
    /* words: number of words in the vocabulary */
    /* size: dimension of the vectors of each words */
    FILE *output_fp;        // Output NN file pointer
    float *M;
    /* M contains all the vectors */
    char *vocab;
    int option;

    // compulsary parameters check
    short bin_file_arg = 0;
    short output_file_arg = 0;
    short mandatory_arg_count = 0;

    output_fp = stdout; // default output medium

    while ((option = getopt(argc, argv,"i:o:t:k:")) != -1) {
        switch (option) {
        case 'i' : // FILE.bin
            strcpy(bin_file, optarg);
            i++;
            bin_file_arg = 1;
            mandatory_arg_count ++;
            break;
        case 'k' : // number of NNs to compute
            N = atoi(optarg);
            i++;
            break;
        case 'o' : // output.txt
            output_fp = fopen(optarg , "w");
            i++;
            output_file_arg = 1;
            break;
        default: print_usage(); 
            exit(EXIT_FAILURE);
        }
    }

    if(mandatory_arg_count != 1)
    {
        print_usage(); 
        printf("Mandatory parameter missing. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    bin_file_fp = fopen(bin_file, "rb");

    bestw = (char**)malloc(N * sizeof(char*));
    bestd = (float*)malloc(N * sizeof(float*));
    if (bin_file_fp == NULL) {
        printf("Input file not found\n");
        return -1;
    }
    fscanf(bin_file_fp, "%lld", &words);
    fscanf(bin_file_fp, "%lld", &size);
    vocab = (char *)malloc((long long)words * max_w * sizeof(char));
    for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
    M = (float *)malloc((long long)words * (long long)size * sizeof(float));
    if (M == NULL) {
        printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
        return -1;
    }
    for (b = 0; b < words; b++) {
        a = 0;
        while (1) {
            vocab[b * max_w + a] = fgetc(bin_file_fp);
            if (feof(bin_file_fp) || (vocab[b * max_w + a] == ' ')) break;
            if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
        }
        vocab[b * max_w + a] = 0;
        for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, bin_file_fp);
        len = 0;
        for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
        len = sqrt(len);
        for (a = 0; a < size; a++) M[a + b * size] /= len;
    }
    fclose(bin_file_fp);

    //
    /* sprintf(output_filename, "%s.%dNN", bin_file, N); */
    /* if ((output_fp = fopen(output_filename, "w")) == NULL) { */
    /*     printf("Error opening output file for writing\n"); */
    /*     return EXIT_FAILURE; */
    /* } */
    //

    fprintf(output_fp, "%lld %d\n", words, N);

    for (i = 0; i < words; i++) {
        printf("%d:%d Word: %s\n", i+1, words, &vocab[i * max_w]);

        for (a = 0; a < N; a++) bestd[a] = 0;
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        a = 0;
        strcpy(st1, &vocab[i * max_w]);
        /* st1 = the input string */
        fprintf(output_fp, "%s\t", st1);

        cn = 0;
        b = 0;
        c = 0;
        while (1) {
            st[cn][b] = st1[c];
            b++;
            c++;
            st[cn][b] = 0;
            if (st1[c] == 0) break;
            if (st1[c] == ' ') {
                cn++;
                b = 0;
                c++;
            }
        }
        cn++;
        /* cn = number of words given as input */
        /* st[][] has each of the words */
        for (a = 0; a < cn; a++) {
            for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
            if (b == words) b = -1;
            bi[a] = b;
//            printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
            if (b == -1) {
                printf("Out of dictionary word!\n");
                break;
            }
        }
        /* bi[] contains the position of the word(s) that are given as input */
        if (b == -1) continue;
        for (a = 0; a < size; a++) vec[a] = 0;
        for (b = 0; b < cn; b++) {
            if (bi[b] == -1) continue;
            for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
        }
        /* simple addition of the vectors done */
        /* composed vector in 'vec' */
        len = 0;
        for (a = 0; a < size; a++) len += vec[a] * vec[a];
        len = sqrt(len);
        for (a = 0; a < size; a++) vec[a] /= len;
        /* Length normalization of the composed vector */
        for (a = 0; a < N; a++) bestd[a] = -1;
        /* all 'N' distances are initialized with -1 */
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        /* all 'N' words are initialized with null */
        for (c = 0; c < words; c++) {
            a = 0;
            for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
            if (a == 1) continue;
            /* same words are ignored */
            dist = 0;
            for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
            /* vector distance of the composed vector(vec) and a candidate vector(M[]) calculated */
            for (a = 0; a < N; a++) {
                if (dist > bestd[a]) {
                    for (d = N - 1; d > a; d--) {
                        bestd[d] = bestd[d - 1];
                        strcpy(bestw[d], bestw[d - 1]);
                    }
                    bestd[a] = dist;
                    strcpy(bestw[a], &vocab[c * max_w]);
                    break;
                }
            }
        }
        for (a = 0; a < N; a++) 
            fprintf(output_fp, "%s:%f\t", bestw[a], bestd[a]);
        fprintf(output_fp, "\n");
    }
    fclose(output_fp);

    return 0;
}
