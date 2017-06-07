//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

const long long max_size = 2000;         // max length of strings
int N = 30;                  // number of closest words that will be shown
const long long max_w = 1024;              // max length of vocabulary entries

int main(int argc, char **argv) {
    FILE *f;
	FILE *root_fp;
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
    char file_name[max_size], st[100][max_size];
	char root_words_file_name[max_size];
    float dist, len/*, bestd[N]*/, vec[max_size];
    long long words, size, a, b, c, d, cn, bi[100], i;
    /* words: number of words in the vocabulary */
    /* size: dimension of the vectors of each words */
    char ch;
    float *M;
    /* M contains all the vectors */
    char *vocab;
    if (argc < 3) {
        printf("Usage: ./distance <FILE.bin> <root-words> [number of closest words; default 30]\nwhere FILE contains word projections in the BINARY FORMAT\n");
        return 0;
    }
    strcpy(file_name, argv[1]);
    strcpy(root_words_file_name, argv[2]);
    f = fopen(file_name, "rb");
    root_fp = fopen(root_words_file_name, "rb");
    if (argc == 4) {
        N = atoi(argv[3]);
    }
	// +++
	term_flag = 0;
	root_word_count = 0;
	do {
		ch = fgetc(root_fp);
		//printf("%c", ch);
		if((ch == ' ' || ch == '\t' || ch == '\n' || ch == EOF) && term_flag == 1) {
			root_word_count++;
			term_flag = 0;
	    }
	    else
	        term_flag = 1;
	} while (ch!=EOF);
	//printf("%d\n", root_word_count);
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
		if((ch == ' ' || ch == '\t' || ch == '\n' || ch == EOF) && term_flag == 1) {
		    root_term[count] = '\0';
			strcpy(root_terms[--root_word_count], root_term);
			count = 0;
			term_flag = 0;
	    }
	    else {
	        root_term[count++] = ch;
	        term_flag = 1;
	    }
	} while (ch!=EOF);

//    for (i=0; i<root_word_count2; i++)
//        printf("%s\n", root_terms[i]);
	// ---


    bestw = (char**)malloc(N * sizeof(char*));
    bestd = (float*)malloc(N * sizeof(float*));
    if (f == NULL) {
        printf("Input file not found\n");
        return -1;
    }
    fscanf(f, "%lld", &words);
    fscanf(f, "%lld", &size);
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
            vocab[b * max_w + a] = fgetc(f);
            if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
            if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
        }
        vocab[b * max_w + a] = 0;
        for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
        len = 0;
        for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
        len = sqrt(len);
        for (a = 0; a < size; a++) M[a + b * size] /= len;
    }
    fclose(f);
    for (i = 0; i < root_word_count2; i++) {

        for (a = 0; a < N; a++) bestd[a] = 0;
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        a = 0;
        strcpy(st1, root_terms[i]);
        //* st1 = the input string 
		printf("%s\t", st1);
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
                printf("Out of dictionary word!\n");
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
        for (a = 0; a < N; a++) printf("%s %f\t", bestw[a], bestd[a]);
        printf("\n");
    }

	return 0;
}
