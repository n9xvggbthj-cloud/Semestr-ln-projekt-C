#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "music.h"

static void print_menu(void)
{
    puts("\n=== Music Library / Mini DJ ===");
    puts("1) Add song");
    puts("2) List all songs");
    puts("3) Search (by title or artist)");
    puts("4) Sort library");
    puts("   1=Title, 2=Artist, 3=Genre, 4=Camelot number, 5=Year");
    puts("5) Save to CSV");
    puts("6) Load from CSV");
    puts("9) Toggle subscription (Mixtape ON/OFF)");
    puts("10) Make mixtape");
    puts("0) Exit");
    printf("Choice: ");
}

static void flush_stdin_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

static void ensure_csv_extension(char *path, size_t cap)
{
    size_t L = strlen(path);
    if (L >= 4)
    {
        const char *ext = path + (L - 4);
        if (ext[0] == '.' && (ext[1] == 'c' || ext[1] == 'C') && (ext[2] == 's' || ext[2] == 'S') && (ext[3] == 'v' || ext[3] == 'V'))
            return;
    }
    if (L + 4 < cap)
        strcat(path, ".csv");
}

int main(void)
{
    Song library[MAX_SONGS];
    int count = 0;
    bool subscribed = false;

    for (;;)
    {
        print_menu();
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            flush_stdin_line();
            continue;
        }
        flush_stdin_line();
        if (choice == 0)
            break;

        switch (choice)
        {
        case 1:
            add_song(library, &count);
            break;
        case 2:
            list_songs(library, count);
            break;
        case 3:
            search_song(library, count);
            break;

        case 4:
        {
            int mode;
            printf("Sort mode: ");
            if (scanf("%d", &mode) != 1)
            {
                flush_stdin_line();
                break;
            }
            flush_stdin_line();
            sort_songs(library, count, mode);
            puts("Sorted.");
        }
        break;

        case 5:
        {
            char path[1024];
            printf("Zadej název/cestu pro uložení CSV: ");
            if (!fgets(path, sizeof(path), stdin))
            {
                puts("Načtení názvu se nezdařilo.");
                break;
            }
            path[strcspn(path, "\n")] = '\0';
            if (!path[0])
                strcpy(path, "library.csv");
            else
                ensure_csv_extension(path, sizeof(path));
            puts(save_csv(library, count, path) ? "Saved." : "Save failed.");
        }
        break;

        case 6:
        {
            char path[1024];
            printf("Zadej cestu k CSV souboru: ");
            if (!fgets(path, sizeof(path), stdin))
            {
                puts("Načtení cesty se nezdařilo.");
                break;
            }
            path[strcspn(path, "\n")] = '\0';
            if (!path[0])
            {
                puts("Nebyla zadána žádná cesta.");
                break;
            }

            if (load_csv(library, &count, path))
            {
                printf("Loaded. (celkem %d skladeb)\n", count);
            }
            else
            {
                puts("Load failed.");
            }
        }
        break;

        case 9:
            subscribed = !subscribed;
            printf("Subscription: %s\n", subscribed ? "ON" : "OFF");
            break;

        case 10:
        {
            if (!subscribed)
            {
                puts("Mixtape je dostupný jen s předplatným.");
                break;
            }
            if (count <= 0)
            {
                puts("Knihovna je prázdná.");
                break;
            }

            int mins = 60;
            int m;
            printf("Cílová délka (minuty) [60]: ");
            if (scanf("%d", &m) == 1 && m > 0)
                mins = m;
            flush_stdin_line();

            int seed_id = -1;
            printf("Zadej ID prvni skladby (1..%d), nebo 0 pro automat: ", count);
            if (scanf("%d", &seed_id) != 1)
                seed_id = 0;
            flush_stdin_line();
            int seed_index = (seed_id >= 1 && seed_id <= count) ? (seed_id - 1) : -1;

            int end_id = -1;
            printf("Zadej ID posledni skladby (1..%d), nebo 0 = bez povinného závěru: ", count);
            if (scanf("%d", &end_id) != 1)
                end_id = 0;
            flush_stdin_line();
            int end_index = (end_id >= 1 && end_id <= count) ? (end_id - 1) : -1;

            Song set[MAX_SONGS];
            int nset = 0;
            int total = generate_mixtape(library, count, mins * 60,
                                         seed_index, end_index,
                                         set, &nset);
            if (nset == 0)
            {
                puts("Nenalezen žádný kompatibilní set.");
                break;
            }

            printf("Mixtape (%d skladeb, %d s ≈ %.1f min)\n", nset, total, total / 60.0);
            printf("%-4s | %-55s | %-20s | %-4s | %-4s | %-4s\n", "No.", "Title", "Artist", "Key", "BPM", "Len");
            for (int i = 0; i < nset; i++)
            {
                printf("%-4d | %-55s | %-20s | %-4s | %-4d | %-4ds\n",
                       i + 1, set[i].title, set[i].artist, set[i].key, set[i].bpm, set[i].length);
            }

            char path[1024];
            printf("Uložit set do CSV? Zadej název (Enter=skip): ");
            if (fgets(path, sizeof(path), stdin))
            {
                path[strcspn(path, "\n")] = '\0';
                if (path[0])
                {
                    ensure_csv_extension(path, sizeof(path));
                    FILE *f = fopen(path, "w");
                    if (f)
                    {
                        fprintf(f, "Title,Artist,Genre,Key,Year,Length,BPM\n");
                        for (int i = 0; i < nset; i++)
                        {
                            fprintf(f, "\"%s\",\"%s\",\"%s\",%s,%d,%d,%d\n",
                                    set[i].title, set[i].artist, set[i].genre, set[i].key,
                                    set[i].year, set[i].length, set[i].bpm);
                        }
                        fclose(f);
                        printf("Uloženo do '%s'.\n", path);
                    }
                    else
                        puts("Uložení selhalo.");
                }
            }
        }
        break;

        default:
            puts("Invalid choice.");
        }
    }

    puts("Bye.");
    return 0;
}