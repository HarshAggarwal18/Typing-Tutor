#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <conio.h>
#include <windows.h>

#define MAX_USERS 100
#define MAX_SENTENCES 5
#define MAX_WORDS 20
#define KEYBOARD_ROWS 3
#define HASH_SIZE 100
#define TIMED_PARAGRAPH_TIME 600
#define FILENAME "users.dat"

// ANSI Color Codes
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define RESET "\x1B[0m"

/* ================= Data Structure Definitions ================== */

typedef struct Session
{
    time_t timestamp;
    float accuracy;
    int wpm;
    int errors;
    struct Session *next;
} Session;

typedef struct GameRecord
{
    char type[20];
    int score;
    time_t timestamp;
    struct GameRecord *next;
} GameRecord;

typedef struct WordTree
{
    char word[50];
    struct WordTree *left;
    struct WordTree *right;
} WordTree;

typedef struct User
{
    char username[50];
    char password[50];
    Session *sessions;
    GameRecord *gameHistory;
    WordTree *wordList;
    struct User *next;
} User;

/* ================= Global Variables ================= */
User *usersHashTable[HASH_SIZE] = {NULL};
User *currentUser = NULL;
int keyStates[256] = {0};

const char *keyboard[KEYBOARD_ROWS] = {
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"};

const char *sentences[MAX_SENTENCES] = {
    "The quick brown fox jumps over the lazy dog",
    "Programming in C is efficient and powerful",
    "Practice makes perfect in typing skills",
    "Typing tutors help improve speed and accuracy",
    "Hello world from the typing tutor program"};

const char *practiceWords[MAX_WORDS] = {
    "computer", "keyboard", "algorithm", "structure", "program",
    "language", "developer", "console", "memory", "pointer",
    "function", "variable", "compile", "execute", "debug",
    "syntax", "runtime", "library", "header", "interface"};

/* ================= Function Prototypes ================= */
unsigned hash(const char *username);
void saveUserData();
void loadUserData();
void registerUser();
int loginUser();
void deleteUser();
void listAllUsers();
void practiceWord();
void practiceSentence();
void wordRace();
void timedParagraph();
void showStats();
void displayKeyboard(const char *input, const char *target);
WordTree *createWordNode(const char *word);
void insertWord(WordTree **root, const char *word);
void addSession(User *user, float accuracy, int wpm, int errors);
void addGameRecord(User *user, const char *type, int score);

/* ================= Core Functionality ================= */

unsigned hash(const char *username) // ->username to hsah value
{
    unsigned hash = 0;
    for (; *username; username++)
        hash = (hash << 5) + *username;
    return hash % HASH_SIZE;
}

WordTree *createWordNode(const char *word)
{
    WordTree *newNode = (WordTree *)malloc(sizeof(WordTree));
    strcpy(newNode->word, word);
    newNode->left = newNode->right = NULL;
    return newNode;
}

void insertWord(WordTree **root, const char *word)
{
    if (!*root)
    {
        *root = createWordNode(word);
    }
    else
    {
        // Str reutn -ve,0,+ive based on match

        int cmp = strcmp(word, (*root)->word);
        if (cmp < 0)
            insertWord(&(*root)->left, word);
        else if (cmp > 0)
            insertWord(&(*root)->right, word);
    }
}

void addSession(User *user, float accuracy, int wpm, int errors)
{
    Session *newSession = (Session *)malloc(sizeof(Session));
    newSession->timestamp = time(NULL);
    newSession->accuracy = accuracy;
    newSession->wpm = wpm;
    newSession->errors = errors;
    // adding to front
    newSession->next = user->sessions;
    user->sessions = newSession;
}

void addGameRecord(User *user, const char *type, int score)
{
    GameRecord *newRecord = (GameRecord *)malloc(sizeof(GameRecord));
    strcpy(newRecord->type, type);
    newRecord->score = score;
    newRecord->timestamp = time(NULL);
    // adding to front
    newRecord->next = user->gameHistory;
    user->gameHistory = newRecord;
}

void saveUserData()
{
    FILE *fp = fopen(FILENAME, "wb");
    if (!fp)
        return;

    for (int i = 0; i < HASH_SIZE; i++)
    {
        User *user = usersHashTable[i];
        while (user)
        {
            fwrite(user, sizeof(User), 1, fp);
            user = user->next;
        }
    }
    fclose(fp);
}

void loadUserData()
{
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp)
        return;

    User *user;
    while ((user = (User *)malloc(sizeof(User))) && fread(user, sizeof(User), 1, fp))
    {
        unsigned index = hash(user->username);
        user->next = usersHashTable[index];
        usersHashTable[index] = user;
    }
    fclose(fp);
}

/* ================= User Management ================== */

void registerUser()
{
    char username[50], password[50];

    printf("Enter username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    unsigned index = hash(username);
    User *user = usersHashTable[index];
    while (user)
    {
        if (strcmp(user->username, username) == 0)
        {
            printf("Username exists!\n");
            return;
        }
        user = user->next;
    }

    User *newUser = (User *)malloc(sizeof(User));
    strcpy(newUser->username, username);

    printf("Enter password: ");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0;
    strcpy(newUser->password, password);

    newUser->sessions = NULL;
    newUser->gameHistory = NULL;
    newUser->wordList = NULL;
    newUser->next = usersHashTable[index];
    usersHashTable[index] = newUser;

    for (int i = 0; i < MAX_WORDS; i++)
        insertWord(&newUser->wordList, practiceWords[i]);

    printf("Registration successful!\n");
    saveUserData();
}

int loginUser()
{
    char username[50], password[50];

    printf("Username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0;

    unsigned index = hash(username);
    User *user = usersHashTable[index];
    while (user)
    {
        if (strcmp(user->username, username) == 0 &&
            strcmp(user->password, password) == 0)
        {
            currentUser = user;
            return 1;
        }
        user = user->next;
    }
    return 0;
}

void deleteUser()
{
    if (!currentUser)
        return;

    char username[50], password[50];
    printf("Enter username to confirm: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Enter password: ");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0;

    if (strcmp(currentUser->username, username) != 0 ||
        strcmp(currentUser->password, password) != 0)
    {
        printf("Authentication failed!\n");
        return;
    }

    unsigned index = hash(username);
    User **pp = &usersHashTable[index];
    while (*pp)
    {
        if (strcmp((*pp)->username, username) == 0)
        {
            *pp = (*pp)->next;
            saveUserData();
            printf("Account deleted!\n");
            currentUser = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

void listAllUsers()
{
    printf("\nRegistered Users:\n");
    for (int i = 0; i < HASH_SIZE; i++)
    {
        User *user = usersHashTable[i];
        while (user)
        {
            printf("- %s\n", user->username);
            user = user->next;
        }
    }
    system("pause");
}

/* ================= Practice Modes ================= */

void displayKeyboard(const char *input, const char *target)
{
    system("cls");
    printf("Target: %s\n\n", target);

    for (int row = 0; row < KEYBOARD_ROWS; row++)
    {
        printf("  ");
        for (int col = 0; col < strlen(keyboard[row]); col++)
        {
            char key = keyboard[row][col];
            int pressed = keyStates[toupper(key)];
            int correct = -1;

            for (size_t i = 0; i < strlen(input); i++)
            {
                if (toupper(input[i]) == key)
                {
                    correct = (input[i] == target[i]) ? 1 : 0;
                    break;
                }
            }

            if (correct == 1)
                printf(GREEN "%c " RESET, key);
            else if (correct == 0)
                printf(RED "%c " RESET, key);
            else if (pressed)
                printf(GREEN "%c " RESET, key);
            else
                printf("%c ", key);
        }
        printf("\n");
    }
    printf("\n");
}

void practiceWord()
{
    system("cls");
    const char *target = practiceWords[rand() % MAX_WORDS];
    char input[50] = {0};
    int errors = 0;
    time_t start = time(NULL);

    printf("Type the word: %s\n", target);

    int pos = 0;
    while (pos < strlen(target))
    {
        if (_kbhit())
        {
            char c = _getch();
            keyStates[toupper(c)] = 2;

            if (c == 8 && pos > 0)
                pos--;
            else if (isalpha(c))
                input[pos++] = c;

            input[pos] = '\0';
            displayKeyboard(input, target);
            printf("Your input: %s\n", input);
        }
    }

    time_t duration = time(NULL) - start;
    int wpm = duration ? (int)((pos / 5) / (duration / 60.0)) : 0;
    for (int i = 0; i < pos; i++)
        if (input[i] != target[i])
            errors++;

    float accuracy = ((pos - errors) / (float)pos) * 100;
    addSession(currentUser, accuracy, wpm, errors);
    printf("\nWPM: %d, Accuracy: %.2f%%\n", wpm, accuracy);
    system("pause");
}

void practiceSentence()
{
    system("cls");
    const char *target = sentences[rand() % MAX_SENTENCES];
    char input[500] = {0};
    int errors = 0;
    time_t start = time(NULL);

    printf("Type the sentence: %s\n", target);

    int pos = 0;
    while (pos < strlen(target))
    {
        if (_kbhit())
        {
            char c = _getch();
            keyStates[toupper(c)] = 2;

            if (c == 8 && pos > 0)
                pos--;
            else if (isprint(c))
                input[pos++] = c;

            input[pos] = '\0';
            displayKeyboard(input, target);
            printf("Your input: ");
            for (int i = 0; i < pos; i++)
            {
                printf("%s%c", input[i] == target[i] ? GREEN : RED, input[i]);
            }
            printf(RESET "\n");
        }
    }

    time_t duration = time(NULL) - start;
    int wpm = duration ? (int)((pos / 5) / (duration / 60.0)) : 0;
    for (int i = 0; i < pos; i++)
        if (input[i] != target[i])
            errors++;

    float accuracy = ((pos - errors) / (float)pos) * 100;
    addSession(currentUser, accuracy, wpm, errors);
    printf("\nWPM: %d, Accuracy: %.2f%%\n", wpm, accuracy);
    system("pause");
}

/* ================= Game Modes ================= */

void wordRace()
{
    system("cls");
    printf("=== Word Race ===\n");
    int score = 0;
    time_t start = time(NULL);

    while (time(NULL) - start < 60)
    {
        const char *word = practiceWords[rand() % MAX_WORDS];
        printf("\nType: %s\n", word);

        char input[50] = {0};
        int pos = 0;
        time_t wordStart = time(NULL);

        while (time(NULL) - wordStart < 1)
        {
            if (_kbhit())
            {
                char c = _getch();
                if (c == 8 && pos > 0)
                    pos--;
                else if (isprint(c))
                    input[pos++] = c;
                input[pos] = '\0';
                printf("\rInput: %s", input);
            }
        }

        if (strcmp(input, word) == 0)
            score += strlen(word);
    }

    addGameRecord(currentUser, "Word Race", score);
    printf("\nFinal Score: %d\n", score);
    system("pause");
}

void timedParagraph()
{
    system("cls");
    const char *target = sentences[rand() % MAX_SENTENCES];
    char input[500] = {0};
    int errors = 0;
    time_t start = time(NULL);

    printf("Type within %d seconds:\n%s\n", TIMED_PARAGRAPH_TIME, target);

    int pos = 0;
    while ((time(NULL) - start < TIMED_PARAGRAPH_TIME) && pos < strlen(target))
    {
        if (_kbhit())
        {
            char c = _getch();
            if (c == 8 && pos > 0)
                pos--;
            else if (isprint(c))
                input[pos++] = c;

            input[pos] = '\0';
            printf("\rCurrent input: %s", input);
        }
    }

    time_t duration = time(NULL) - start;
    int wpm = duration ? (int)((pos / 5) / (duration / 60.0)) : 0;
    for (int i = 0; i < pos; i++)
        if (input[i] != target[i])
            errors++;

    float accuracy = ((pos - errors) / (float)pos) * 100;
    addSession(currentUser, accuracy, wpm, errors);
    printf("\nTime's up!\nWPM: %d, Accuracy: %.2f%%\n", wpm, accuracy);
    system("pause");
}

/* ================= Statistics ================= */

void showStats()
{
    system("cls");
    printf("=== Statistics ===\n");
    Session *session = currentUser->sessions;
    while (session)
    {
        struct tm *timeinfo = localtime(&session->timestamp);
        printf("[%s] WPM: %d | Accuracy: %.2f%% | Errors: %d\n",
               asctime(timeinfo), session->wpm, session->accuracy, session->errors);
        session = session->next;
    }
    system("pause");
}

/* ================= Main Application ================= */

int main()
{
    srand(time(NULL));
    loadUserData();

    while (1)
    {
        system("cls");
        printf("Main Menu\n1. Login\n2. Register\n3. List Users\n4. Exit\nChoice: ");
        char choice = _getch();

        if (choice == '4')
            break;

        if (choice == '2')
        {
            registerUser();
            system("pause"); // press key to continue
        }
        else if (choice == '1')
        {
            if (loginUser())
            {
                while (currentUser)
                {
                    system("cls");
                    printf("Welcome %s!\n1. Practice\n2. Games\n3. Stats\n4. Delete Account\n5. Logout\nChoice: ",
                           currentUser->username);
                    choice = _getch();

                    if (choice == '5')
                        break;

                    if (choice == '1')
                    {
                        system("cls");
                        printf("1. Words\n2. Sentences\nChoice: ");
                        choice = _getch();
                        if (choice == '1')
                            practiceWord();
                        else if (choice == '2')
                            practiceSentence();
                    }
                    else if (choice == '2')
                    {
                        system("cls");
                        printf("1. Word Race\n2. Timed Paragraph\nChoice: ");
                        choice = _getch();
                        if (choice == '1')
                            wordRace();
                        else if (choice == '2')
                            timedParagraph();
                    }
                    else if (choice == '3')
                        showStats();
                    else if (choice == '4')
                        deleteUser();
                }
            }
            else
            {
                printf("Invalid credentials!\n");
                system("pause");
            }
        }
        else if (choice == '3')
            listAllUsers();
    }

    saveUserData();
    return 0;
}