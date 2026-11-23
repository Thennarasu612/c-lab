/*
 * Railway Management System - Mini Project (C + Text File)
 *
 * Admin side : add train, update timing, update ticket cost, delete train
 * User side  : view all trains, check timing & cost, search by route
 *
 * Data is stored in "trains.txt" (one train per line, fields separated by |)
 * Format: train_no|train_name|source|destination|departure|arrival|cost
 *
 * Compile : gcc railway.c -o railway
 * Run     : ./railway        (Windows: railway.exe)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DATA_FILE  "trains.txt"
#define ADMIN_PASS "spidy"
#define MAX_TRAINS 200

typedef struct {
    int    no;
    char   name[50];
    char   src[30];
    char   dest[30];
    char   dep[6];     /* HH:MM */
    char   arr[6];     /* HH:MM */
    double cost;
} Train;

Train trains[MAX_TRAINS];
int   count = 0;

/* Read one line safely (removes the newline) */
void read_line(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL)
        buf[0] = '\0';
    buf[strcspn(buf, "\n")] = '\0';
}

/* '|' is our field separator, so it cannot be part of the text */
void clean(char *s)
{
    for (; *s; s++)
        if (*s == '|')
            *s = ' ';
}

/* Check HH:MM format and range. Returns 1 if valid */
int valid_time(const char *t)
{
    int h, m;
    char extra;
    if (sscanf(t, "%d:%d%c", &h, &m, &extra) != 2)
        return 0;
    return h >= 0 && h <= 23 && m >= 0 && m <= 59;
}

/* Convert "7:5" -> "07:05" */
void fix_time(const char *in, char *out)
{
    int h, m;
    sscanf(in, "%d:%d", &h, &m);
    sprintf(out, "%02d:%02d", h, m);
}

/* Case-insensitive compare */
int same_text(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

int find_train(int no)
{
    for (int i = 0; i < count; i++)
        if (trains[i].no == no)
            return i;
    return -1;
}

/* ---------- File handling ---------- */
void save_trains(void)
{
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        printf("Error: cannot write to %s\n", DATA_FILE);
        return;
    }
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%.2f\n",
                trains[i].no, trains[i].name, trains[i].src, trains[i].dest,
                trains[i].dep, trains[i].arr, trains[i].cost);
    fclose(fp);
}

void load_trains(void)
{
    FILE *fp = fopen(DATA_FILE, "r");
    char line[300];

    count = 0;
    if (fp == NULL)
        return;     /* first run: no file yet, start empty */

    while (count < MAX_TRAINS && fgets(line, sizeof(line), fp)) {
        Train t;
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%29[^|]|%5[^|]|%5[^|]|%lf",
                   &t.no, t.name, t.src, t.dest, t.dep, t.arr, &t.cost) == 7)
            trains[count++] = t;
    }
    fclose(fp);
}

/* ---------- Display ---------- */
void print_header(void)
{
    printf("\n%-8s %-22s %-12s %-12s %-8s %-8s %-9s\n",
           "Train No", "Name", "From", "To", "Departs", "Arrives", "Cost(Rs)");
    printf("--------------------------------------------------------------------------------\n");
}

void print_train(const Train *t)
{
    printf("%-8d %-22s %-12s %-12s %-8s %-8s %-9.2f\n",
           t->no, t->name, t->src, t->dest, t->dep, t->arr, t->cost);
}

void show_all(void)
{
    if (count == 0) {
        printf("No trains found.\n");
        return;
    }
    print_header();
    for (int i = 0; i < count; i++)
        print_train(&trains[i]);
}

/* ================= ADMIN SIDE ================= */
void add_train(void)
{
    char no[20], cost[20], dep[20], arr[20];
    Train t;

    read_line("Train number : ", no, sizeof(no));
    t.no = atoi(no);
    if (t.no <= 0) {
        printf("Invalid train number!\n");
        return;
    }
    if (find_train(t.no) != -1) {
        printf("Train number already exists!\n");
        return;
    }
    if (count >= MAX_TRAINS) {
        printf("Train list is full!\n");
        return;
    }

    read_line("Train name : ", t.name, sizeof(t.name));
    read_line("Source station : ", t.src, sizeof(t.src));
    read_line("Destination station : ", t.dest, sizeof(t.dest));
    read_line("Departure (HH:MM) : ", dep, sizeof(dep));
    read_line("Arrival (HH:MM) : ", arr, sizeof(arr));
    read_line("Ticket cost (Rs) : ", cost, sizeof(cost));

    if (t.name[0] == '\0' || t.src[0] == '\0' || t.dest[0] == '\0') {
        printf("Name, source and destination cannot be empty!\n");
        return;
    }
    if (!valid_time(dep) || !valid_time(arr)) {
        printf("Invalid time! Use HH:MM (24-hour), example 06:15\n");
        return;
    }
    t.cost = atof(cost);
    if (t.cost <= 0) {
        printf("Invalid ticket cost!\n");
        return;
    }

    clean(t.name);
    clean(t.src);
    clean(t.dest);
    fix_time(dep, t.dep);
    fix_time(arr, t.arr);

    trains[count++] = t;
    save_trains();
    printf("Train added successfully!\n");
}

void update_time(void)
{
    char no[20], dep[20], arr[20];
    int i;

    read_line("Train number to update : ", no, sizeof(no));
    i = find_train(atoi(no));
    if (i == -1) {
        printf("No train found with that number.\n");
        return;
    }
    read_line("New departure (HH:MM) : ", dep, sizeof(dep));
    read_line("New arrival (HH:MM) : ", arr, sizeof(arr));
    if (!valid_time(dep) || !valid_time(arr)) {
        printf("Invalid time! Use HH:MM (24-hour), example 06:15\n");
        return;
    }
    fix_time(dep, trains[i].dep);
    fix_time(arr, trains[i].arr);
    save_trains();
    printf("Train timing updated!\n");
}

void update_cost(void)
{
    char no[20], cost[20];
    int i;
    double c;

    read_line("Train number to update : ", no, sizeof(no));
    i = find_train(atoi(no));
    if (i == -1) {
        printf("No train found with that number.\n");
        return;
    }
    read_line("New ticket cost (Rs) : ", cost, sizeof(cost));
    c = atof(cost);
    if (c <= 0) {
        printf("Invalid ticket cost!\n");
        return;
    }
    trains[i].cost = c;
    save_trains();
    printf("Ticket cost updated!\n");
}

void delete_train(void)
{
    char no[20];
    int i;

    read_line("Train number to delete : ", no, sizeof(no));
    i = find_train(atoi(no));
    if (i == -1) {
        printf("No train found with that number.\n");
        return;
    }
    for (int j = i; j < count - 1; j++)
        trains[j] = trains[j + 1];
    count--;
    save_trains();
    printf("Train deleted!\n");
}

void admin_menu(void)
{
    char pass[50], choice[10];

    read_line("Enter admin password: ", pass, sizeof(pass));
    if (strcmp(pass, ADMIN_PASS) != 0) {
        printf("Wrong password!\n");
        return;
    }
    while (1) {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. Add new train\n");
        printf("2. Update train timing\n");
        printf("3. Update ticket cost\n");
        printf("4. Delete train\n");
        printf("5. View all trains\n");
        printf("6. Back\n");
        read_line("Enter choice: ", choice, sizeof(choice));
        switch (atoi(choice)) {
            case 1: add_train(); break;
            case 2: update_time(); break;
            case 3: update_cost(); break;
            case 4: delete_train(); break;
            case 5: show_all(); break;
            case 6: return;
            default: printf("Invalid choice!\n");
        }
    }
}

/* ================= USER SIDE ================= */
void search_by_number(void)
{
    char no[20];
    int i;

    read_line("Enter train number: ", no, sizeof(no));
    i = find_train(atoi(no));
    if (i == -1) {
        printf("No trains found.\n");
        return;
    }
    print_header();
    print_train(&trains[i]);
}

void search_by_route(void)
{
    char src[40], dest[40];
    int found = 0;

    read_line("From station : ", src, sizeof(src));
    read_line("To station : ", dest, sizeof(dest));

    for (int i = 0; i < count; i++) {
        if (same_text(trains[i].src, src) && same_text(trains[i].dest, dest)) {
            if (!found)
                print_header();
            print_train(&trains[i]);
            found = 1;
        }
    }
    if (!found)
        printf("No trains found.\n");
}

void user_menu(void)
{
    char choice[10];

    while (1) {
        printf("\n===== USER MENU =====\n");
        printf("1. View all trains (timings & cost)\n");
        printf("2. Check by train number\n");
        printf("3. Search by route (From - To)\n");
        printf("4. Back\n");
        read_line("Enter choice: ", choice, sizeof(choice));
        switch (atoi(choice)) {
            case 1: show_all(); break;
            case 2: search_by_number(); break;
            case 3: search_by_route(); break;
            case 4: return;
            default: printf("Invalid choice!\n");
        }
    }
}

/* ================= MAIN ================= */
int main(void)
{
    char choice[10];

    load_trains();

    while (1) {
        printf("\n=====================================\n");
        printf("     RAILWAY MANAGEMENT SYSTEM\n");
        printf("=====================================\n");
        printf("1. Admin (Train Management)\n");
        printf("2. User (Check Timing & Cost)\n");
        printf("3. Exit\n");
        read_line("Enter choice: ", choice, sizeof(choice));
        switch (atoi(choice)) {
            case 1: admin_menu(); break;
            case 2: user_menu(); break;
            case 3:
                printf("Thank you!\n");
                return 0;
            default: printf("Invalid choice!\n");
        }
    }
}
