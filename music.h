#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

#define MAX_SONGS 5000
#define NAME_LEN 128
#define GENRE_LEN 64
#define KEY_LEN 8 // Camelot číslo 1..12

typedef struct
{
    char title[NAME_LEN];
    char artist[NAME_LEN];
    char genre[GENRE_LEN];
    char key[KEY_LEN]; // Camelot číslo (1–12)
    int year;          // 0 = neznámé
    int length;        // v sekundách
    int bpm;           // 0 = neznámé
} Song;

//Základní funkce
void add_song(Song library[], int *count);
void list_songs(const Song library[], int count);
void search_song(const Song library[], int count);
void sort_songs(Song library[], int count, int mode); // 1=title,2=artist,3=genre,4=key,5=year
bool save_csv(const Song library[], int count, const char *path);
bool load_csv(Song library[], int *count, const char *path);

//Generátor setu
int parse_camelot(const char *k, int *num, char *mode); 
int generate_mixtape(const Song lib[], int count,
                     int target_seconds,
                     int seed_index, 
                     int end_index,  
                     Song out[], int *out_count);

#endif