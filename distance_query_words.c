/*
*    Modified version of distance.c.
*    It creates a file that contains NNs of a list of words given as command line input
*    The default value of K is set to 30.
*    The first line of the output file will contain: <no-of-root-words> <K>
*    Output:
*        <root-term_1>\t<nn_1:sim_1>\t<nn_2:sim_2>\t ... <nn_K:sim_K>\t\n
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <getopt.h>
#include <stdlib.h>

const long long max_size = 2000;         // max length of strings
int N = 30;                  // number of closest words that will be shown (default)
const long long max_w = 1024;              // max length of vocabulary entries

void print_usage() {
    printf("Usage: distance_query_words "
        "-i <FILE.bin: FILE contains word projections in the BINARY FORMAT> "
        "-t <root-words> "
        "[-k <k to calculate kNN (default-30)>] "
        "[-o <output-file (default-stdout)>]\n");
}

int main(int argc, char **argv) {
    FILE *bin_file_fp;
    FILE *root_fp;
    FILE *output_fp;
    char st1[max_size];
    int root_word_count;
    int root_word_count2;
    char **root_terms;
    char root_term[max_w];
    int term_flag;
    int count;
//    char *bestw[N];
    char **bestw;
    float *bestd;
    char bin_file[max_size], st[100][max_size];
    char root_words_file_name[max_size];
    float dist, len/*, bestd[N]*/, vec[max_size];
    long long words, size, a, b, c, d, cn, bi[100], i;
    /* words: number of words in the vocabulary */
    /* size: dimension of the vectors of each words */
    char ch;
    float *M;
    /* M contains all the vectors */
    char *vocab;
    int option;

    // compulsary parameters check
    short bin_file_arg = 0;
    short root_file_arg = 0;
    short output_file_arg = 0;
    short mandatory_arg_count = 0;

    if (argc < 3) {
        /* printf("Usage: ./%s -i <FILE.bin: FILE contains word projections in the BINARY FORMAT> -t <root-words> [-k <k to calculate kNN (default-30)>] [-o <output-file (default-stdout)>]", argv[0]); */
        print_usage();
        return 0;
    }

    output_fp = stdout; // default output medium

    while ((option = getopt(argc, argv,"i:o:t:k:")) != -1) {
        switch (option) {
        case 'i' : // FILE.bin
            strcpy(bin_file, optarg);
            i++;
            bin_file_arg = 1;
            mandatory_arg_count ++;
            break;
        case 't' : // root-words.txt
            strcpy(root_words_file_name, optarg);
            i++;
            root_file_arg = 1;
            mandatory_arg_count ++;
            break;
        case 'k' : // number of NNs to compute; default is 30
            N = atoi(optarg);
            i++;
            break;
        case 'o' : // output.txt; default is STDOUT
            output_fp = fopen(optarg , "w");
            i++;
            output_file_arg = 1;
            break;
        default: print_usage(); 
            exit(EXIT_FAILURE);
        }
    }

    if(mandatory_arg_count != 2)
    {
        print_usage(); 
        printf("Mandatory parameter missing. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    /* strcpy(bin_file, argv[1]); */
    /* strcpy(root_words_bin_file, argv[2]); */
    bin_file_fp = fopen(bin_file, "rb");
    root_fp = fopen(root_words_file_name, "rb");
    /* if (argc == 4) { */
    /*     N = atoi(argv[3]); */
    /* } */
    // +++
    term_flag = 0;
    root_word_count = 0;
    do {
        ch = fgetc(root_fp);
        //printf("%c", ch);
        if((/* ch == ' ' || */ ch == '\t' || ch == '\n' || ch == EOF) && term_flag == 1) {
            root_word_count++;
            term_flag = 0;
        }
        else
            term_flag = 1;
    } while (ch!=EOF);
    printf("Number of root words: %d\n", root_word_count);
    if(fseek(root_fp, 0L, SEEK_SET) != 0)
    {
        printf("Error rewinding the file");
    }
    root_terms = (char**)malloc(sizeof(char*) * root_word_count);
    for(i=0; i<root_word_count; i++)
        root_terms[i] = (char*)malloc(sizeof(char) * max_w);

    root_word_count2 = root_word_count;
    root_term[0] = '\0';
    term_flag = 0;
    count = 0;
    do {
        ch = fgetc(root_fp);
        if((/* ch == ' ' || */ ch == '\t' || ch == '\n' || ch == EOF) && term_flag == 1) {
            root_term[count] = '\0';
            strcpy(root_terms[--root_word_count2], root_term);
            count = 0;
            term_flag = 0;
        }
        else {
            root_term[count++] = ch;
            term_flag = 1;
        }
    } while (ch!=EOF);

//    for (i=0; i<root_word_count; i++)
//        printf("%s\n", root_terms[i]);
    // ---

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

    fprintf(output_fp, "%d %d\n", root_word_count, N);

    for (i = 0; i < root_word_count; i++) {

        for (a = 0; a < N; a++) bestd[a] = 0;
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        a = 0;
        strcpy(st1, root_terms[i]);
        //* st1 = the input string
        if( output_file_arg && (0 == (i+1)%100) )
            printf("Computed NN for %lld out of %d words\n", i+1, root_word_count);
        fprintf(output_fp, "%s\t", st1);
        if (!strcmp(st1, "EXIT")) break;
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
        //* cn = number of words given as input
        //* st[][] has each of the words
        for (a = 0; a < cn; a++) {
            for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
            if (b == words) b = -1;
            bi[a] = b;
//            printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
            if (b == -1) {
                fprintf(output_fp, "OOV\n");
                break;
            }
        }
        //* bi[] contains the position of the word(s) that are given as input
        if (b == -1) continue;
        for (a = 0; a < size; a++) vec[a] = 0;
        for (b = 0; b < cn; b++) {
            if (bi[b] == -1) continue;
            for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
        }
        //* simple addition of the vectors done 
        //* composed vector in 'vec' 
        len = 0;
        for (a = 0; a < size; a++) len += vec[a] * vec[a];
        len = sqrt(len);
        for (a = 0; a < size; a++) vec[a] /= len;
        //* Length normalization of the composed vector
        for (a = 0; a < N; a++) bestd[a] = -1;
        //* all 'N' distances are initialized with -1
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        //* all 'N' words are initialized with null
        for (c = 0; c < words; c++) {
            a = 0;
            for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
            if (a == 1) continue;
            //* same words are ignored 
            dist = 0;
            for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
            //* vector distance of the composed vector(vec) and a candidate vector(M[]) calculated
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
    fclose(root_fp);

    return 0;
}
