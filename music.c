#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "music.h"

static void flush_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
static char *rtrim(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t'))
        s[--n] = 0;
    return s;
}
static char *ltrim(char *s)
{
    while (*s == ' ' || *s == '\t')
        ++s;
    return s;
}
static void trim_inplace(char *s)
{
    char *p = ltrim(s);
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    rtrim(s);
}

static char *str_dup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *p = (char *)malloc(len + 1);
    if (!p)
        return NULL;
    memcpy(p, s, len + 1);
    return p;
}

// Přidání, výpis, hledání

void add_song(Song library[], int *count)
{
    if (*count >= MAX_SONGS)
    {
        puts("Library is full!");
        return;
    }
    Song *s = &library[*count];
    printf("Title: ");
    fgets(s->title, NAME_LEN, stdin);
    rtrim(s->title);
    printf("Artist: ");
    fgets(s->artist, NAME_LEN, stdin);
    rtrim(s->artist);
    printf("Genre: ");
    fgets(s->genre, GENRE_LEN, stdin);
    rtrim(s->genre);
    printf("Camelot key (napr. 8A): ");
    fgets(s->key, KEY_LEN, stdin);
    rtrim(s->key);
    printf("Year: ");
    if (scanf("%d", &s->year) != 1)
        s->year = 0;
    printf("Length (sec): ");
    if (scanf("%d", &s->length) != 1)
        s->length = 0;
    printf("BPM (0=unknown): ");
    if (scanf("%d", &s->bpm) != 1)
        s->bpm = 0;
    flush_line();
    (*count)++;
    puts("Song added.");
}

void list_songs(const Song library[], int count)
{
    if (count == 0)
    {
        puts("Library empty.");
        return;
    }
    printf("%-4s | %-55s | %-20s | %-18s | %-4s | %-4s | %-5s | %-4s\n",
           "ID", "Title", "Artist", "Genre", "Key", "Year", "Len", "BPM");
    for (int i = 0; i < count; i++)
    {
        printf("%-4d | %-55s | %-20s | %-18s | %-4s | %-4d | %-4ds | %-4d\n",
               i + 1, library[i].title, library[i].artist, library[i].genre,
               library[i].key, library[i].year, library[i].length, library[i].bpm);
    }
}

void search_song(const Song library[], int count)
{
    char q[NAME_LEN];
    printf("Search title/artist: ");
    if (!fgets(q, sizeof(q), stdin))
        return;
    rtrim(q);
    if (!*q)
        return;
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (strstr(library[i].title, q) || strstr(library[i].artist, q))
        {
            if (!found)
            {
                printf("%-4s | %-55s | %-20s | %-18s | %-4s | %-4s | %-5s | %-4s\n",
                       "ID", "Title", "Artist", "Genre", "Key", "Year", "Len", "BPM");
            }
            found = 1;
            printf("%-4d | %-55s | %-20s | %-18s | %-4s | %-4d | %-4ds | %-4d\n",
                   i + 1, library[i].title, library[i].artist, library[i].genre,
                   library[i].key, library[i].year, library[i].length, library[i].bpm);
        }
    }
    if (!found)
        puts("No match.");
}

// Řazení

static int cmp_title(const void *a, const void *b) { return strcmp(((const Song *)a)->title, ((const Song *)b)->title); }
static int cmp_artist(const void *a, const void *b) { return strcmp(((const Song *)a)->artist, ((const Song *)b)->artist); }
static int cmp_genre(const void *a, const void *b) { return strcmp(((const Song *)a)->genre, ((const Song *)b)->genre); }
static int cmp_year(const void *a, const void *b) { return ((const Song *)a)->year - ((const Song *)b)->year; }

int parse_camelot(const char *k, int *num, char *mode)
{
    if (!k || !*k)
        return 0;
    int n = 0;
    char m = 'A';
    int got = sscanf(k, "%d %c", &n, &m);
    if (got < 1)
        return 0;
    if (n < 1 || n > 12)
        return 0;

    if (got == 1)
        m = 'A';

    m = (m == 'a') ? 'A' : (m == 'b') ? 'B'
                                      : m;
    if (m != 'A' && m != 'B')
        return 0;
    if (num)
        *num = n;
    if (mode)
        *mode = m;
    return 1;
}

static int cmp_key(const void *a, const void *b)
{
    const Song *sa = (const Song *)a, *sb = (const Song *)b;
    int na = 0, nb = 0;
    if (!parse_camelot(sa->key, &na, NULL))
        na = 0;
    if (!parse_camelot(sb->key, &nb, NULL))
        nb = 0;
    return na - nb;
}

void sort_songs(Song library[], int count, int mode)
{
    switch (mode)
    {
    case 1:
        qsort(library, count, sizeof(Song), cmp_title);
        break;
    case 2:
        qsort(library, count, sizeof(Song), cmp_artist);
        break;
    case 3:
        qsort(library, count, sizeof(Song), cmp_genre);
        break;
    case 4:
        qsort(library, count, sizeof(Song), cmp_key);
        break;
    case 5:
        qsort(library, count, sizeof(Song), cmp_year);
        break;
    default:
        puts("Invalid sort mode.");
    }
}

// CSV parsování

static int split_csv_fields(const char *line, char delim, char *out[], int max_fields)
{
    if (!line || !out || max_fields <= 0)
        return 0;

    char *tmp = str_dup(line);
    if (!tmp)
        return 0;

    rtrim(tmp);

    int n = 0;
    char *p = tmp;

    while (n < max_fields)
    {
        char *start = p;

        while (*p && *p != delim)
            p++;

        if (*p == delim)
        {
            *p = '\0';
            p++;
        }

        trim_inplace(start);
        out[n] = str_dup(start);
        if (!out[n])
        {
            for (int i = 0; i < n; i++)
                free(out[i]);
            free(tmp);
            return 0;
        }

        n++;

        if (*p == '\0')
            break;
    }

    free(tmp);
    return n;
}

static void free_fields(char *arr[], int n)
{
    for (int i = 0; i < n; i++)
        free(arr[i]);
}

bool save_csv(const Song library[], int count, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return false;
    fprintf(f, "Title,Artist,Genre,Key,Year,Length,BPM\n");
    for (int i = 0; i < count; i++)
    {
        const Song *s = &library[i];
        fprintf(f, "\"%s\",\"%s\",\"%s\",%s,%d,%d,%d\n",
                s->title, s->artist, s->genre,
                s->key[0] ? s->key : "", s->year, s->length, s->bpm);
    }
    fclose(f);
    return true;
}

bool load_csv(Song library[], int *count, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return false;
    *count = 0;

    char line[8192];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return true;
    }
    char first[8192];
    strncpy(first, line, sizeof(first) - 1);
    first[sizeof(first) - 1] = 0;

    char delim = ',';
    if (strchr(first, ';') && !strchr(first, ','))
        delim = ';';

    int header_fields_n = 0;
    char *fields[64] = {0};
    header_fields_n = split_csv_fields(first, delim, fields, 64);
    int is_header = 0;
    for (int i = 0; i < header_fields_n; i++)
    {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "%s", fields[i]);
        trim_inplace(tmp);
        for (char *p = tmp; *p; ++p)
            *p = (char)tolower((unsigned char)*p);
        if (strstr(tmp, "title") && strstr(first, "Artist"))
            is_header = 1;
    }
    free_fields(fields, header_fields_n);

    if (!is_header)
    {
        fseek(f, 0, SEEK_SET);
    }

    while (*count < MAX_SONGS && fgets(line, sizeof(line), f))
    {
        rtrim(line);
        if (!*line)
            continue;

        char *arr[64] = {0};
        int n = split_csv_fields(line, delim, arr, 64);
        if (n < 6)
        {
            free_fields(arr, n);
            continue;
        }

        Song *s = &library[*count];

#define COPY_FIELD(dst, srcidx, maxlen)                     \
    do                                                      \
    {                                                       \
        if ((srcidx) < n && arr[(srcidx)])                  \
        {                                                   \
            snprintf((dst), (maxlen), "%s", arr[(srcidx)]); \
            trim_inplace(dst);                              \
        }                                                   \
        else                                                \
            (dst)[0] = '\0';                                \
    } while (0)

        COPY_FIELD(s->title, 0, NAME_LEN);
        COPY_FIELD(s->artist, 1, NAME_LEN);
        COPY_FIELD(s->genre, 2, GENRE_LEN);
        COPY_FIELD(s->key, 3, KEY_LEN);

        s->year = (arr[4] && *arr[4]) ? atoi(arr[4]) : 0;
        s->length = (arr[5] && *arr[5]) ? atoi(arr[5]) : 0;
        s->bpm = (n > 6 && arr[6] && *arr[6]) ? atoi(arr[6]) : 0;

        if (s->genre[0] == '\0')
            snprintf(s->genre, GENRE_LEN, "Unknown");

        free_fields(arr, n);
        (*count)++;
    }

    fclose(f);
    return true;
}

static int wrap_camelot(int n)
{
    while (n < 1)
        n += 12;
    while (n > 12)
        n -= 12;
    return n;
}

static void seed_rng_once(void)
{
    static int seeded = 0;
    if (!seeded)
    {
        seeded = 1;
        srand((unsigned)time(NULL));
    }
}

// Set generátor

int generate_mixtape(const Song lib[], int count,
                     int target_seconds,
                     int seed_index, int end_index,
                     Song out[], int *out_count)
{
    if (!out_count)
        return 0;
    *out_count = 0;
    if (!lib || !out)
        return 0;
    if (count <= 0 || target_seconds <= 0)
        return 0;

    seed_rng_once();

    // Výběr seedu
    int cur = seed_index;
    if (cur < 0 || cur >= count)
    {
        cur = -1;
        for (int i = 0; i < count; i++)
        {
            int n;
            char m;
            if (lib[i].length > 0 && parse_camelot(lib[i].key, &n, &m))
            {
                cur = i;
                break;
            }
        }
    }
    if (cur < 0)
        return 0;

    // Výběr zakončení
    int end = end_index;
    int endNum = 0;
    char endMode = 'A';
    int endLen = 0;
    if (end >= 0)
    {
        if (end >= count)
            return 0;
        if (end == cur)
            return 0;
        if (lib[end].length <= 0)
            return 0;
        if (!parse_camelot(lib[end].key, &endNum, &endMode))
            return 0;
        (void)endMode;
        endLen = lib[end].length;
    }

    bool used[MAX_SONGS] = {0};

    int curNum = 0;
    if (!parse_camelot(lib[cur].key, &curNum, NULL))
        return 0;

    int total = 0;
    out[(*out_count)++] = lib[cur];
    used[cur] = true;
    total += lib[cur].length;

    while (total + endLen < target_seconds && *out_count < MAX_SONGS)
    {

        int dir = (rand() % 2) ? 1 : -1;

        if (end >= 0)
        {
            int diff = (endNum - curNum + 12) % 12;
            int bestDir = (diff == 0) ? ((rand() % 2) ? 1 : -1) : (diff <= 6 ? 1 : -1);

            if ((rand() % 100) < 70)
                dir = bestDir;
            else
                dir = (rand() % 2) ? 1 : -1;
        }

        int pick = -1;

        for (int radius = 1; radius <= 6 && pick < 0; ++radius)
        {
            int candidates[MAX_SONGS];
            int candN = 0;

            int want = wrap_camelot(curNum + dir * radius);

            for (int i = 0; i < count; i++)
            {
                if (used[i])
                    continue;
                if (i == end)
                    continue;
                if (lib[i].length <= 0)
                    continue;

                int n;
                if (!parse_camelot(lib[i].key, &n, NULL))
                    continue;
                if (n != want)
                    continue;

                if (total + lib[i].length + endLen > target_seconds + 30)
                    continue;

                candidates[candN++] = i;
            }

            if (candN == 0)
            {
                want = wrap_camelot(curNum - dir * radius);
                for (int i = 0; i < count; i++)
                {
                    if (used[i])
                        continue;
                    if (i == end)
                        continue;
                    if (lib[i].length <= 0)
                        continue;

                    int n;
                    if (!parse_camelot(lib[i].key, &n, NULL))
                        continue;
                    if (n != want)
                        continue;

                    if (total + lib[i].length + endLen > target_seconds + 30)
                        continue;

                    candidates[candN++] = i;
                }
            }

            if (candN > 0)
            {
                pick = candidates[rand() % candN];
                curNum = want;
            }
        }

        if (pick < 0)
        {
            int candidates[MAX_SONGS];
            int candN = 0;
            for (int i = 0; i < count; i++)
            {
                if (used[i])
                    continue;
                if (i == end)
                    continue;
                if (lib[i].length <= 0)
                    continue;

                int n;
                if (!parse_camelot(lib[i].key, &n, NULL))
                    continue;

                if (total + lib[i].length + endLen > target_seconds + 30)
                    continue;

                candidates[candN++] = i;
            }
            if (candN > 0)
            {
                pick = candidates[rand() % candN];
                parse_camelot(lib[pick].key, &curNum, NULL);
            }
        }

        if (pick < 0)
            break;

        out[(*out_count)++] = lib[pick];
        used[pick] = true;
        total += lib[pick].length;
    }

    if (end >= 0 && *out_count < MAX_SONGS)
    {
        out[(*out_count)++] = lib[end];
        total += lib[end].length;
    }

    return total;
}