CC = gcc
#Using -Ofast instead of -O3 might result in faster code, but is supported only by newer GCC versions
CFLAGS = -lm -pthread -O3 -march=native -Wall -funroll-loops -Wno-unused-result

all: word2vec word2phrase distance distance_all_words distance_query_words word-analogy compute-accuracy vec2bin

vec2bin : vecToBin.c
	$(CC) vecToBin.c -o vecToBin $(CFLAGS)
word2vec : word2vec.c
	$(CC) word2vec.c -o word2vec $(CFLAGS)
word2phrase : word2phrase.c
	$(CC) word2phrase.c -o word2phrase $(CFLAGS)
distance : distance.c
	$(CC) distance.c -o distance $(CFLAGS)
distance_all_words : distance_all_words.c
	$(CC) distance_all_words.c -o distance_all_words $(CFLAGS)
distance_query_words : distance_query_words.c
	$(CC) distance_query_words.c -o distance_query_words $(CFLAGS)
word-analogy : word-analogy.c
	$(CC) word-analogy.c -o word-analogy $(CFLAGS)
compute-accuracy : compute-accuracy.c
	$(CC) compute-accuracy.c -o compute-accuracy $(CFLAGS)
	chmod +x *.sh

clean:
	rm -rf word2vec word2phrase distance distance_all_words distance_query_words word-analogy compute-accuracy
