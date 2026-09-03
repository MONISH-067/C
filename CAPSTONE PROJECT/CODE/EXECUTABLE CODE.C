#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BOOKS 100
#define MAX_USERS 50
#define MAX_BORROWED_PER_USER 5

// --- Configuration Constants ---
#define LOW_STOCK_THRESHOLD 3
#define MEMBERSHIP_DURATION 180  // 3 minutes (Demo purposes)
#define BOOK_BORROW_PERIOD 60    // 1 minute (Demo purposes)
#define FINE_PER_SECOND 2.0      // $2 per overdue second

// --- Data Structures ---
typedef struct {
    int id;
    char title[100];
    char author[50];
    char genre[30];
    int stock;
    int borrow_count;
} Book;

typedef struct {
    int book_id;
    char title[100];
    time_t issue_time;
    time_t due_time;
} BorrowedBook;

typedef struct {
    int id;
    char name[50];
    int is_staff; // 1 for Staff, 0 for Member
    time_t membership_start;
    time_t membership_expiry;
    BorrowedBook borrowed_books[MAX_BORROWED_PER_USER];
    int book_count;
} User;

// --- Global Databases ---
// Main DB: Pre-sorted alphabetically by TITLE for O(log n) search
Book bookDB[MAX_BOOKS] = {
    {1, "A Brief History of Time", "Stephen Hawking", "Science", 2, 15},
    {2, "Clean Code", "Robert C. Martin", "Technology", 5, 42},
    {3, "Dune", "Frank Herbert", "Sci-Fi", 4, 30},
    {4, "The Great Gatsby", "F. Scott Fitzgerald", "Fiction", 1, 10},
    {5, "To Kill a Mockingbird", "Harper Lee", "Fiction", 10, 25}
};
int total_books = 5;

// Index array for O(log n) search by AUTHOR
Book* authorIndex[MAX_BOOKS];

User userDB[MAX_USERS];
int total_users = 0;

// --- Initialization & Sorting ---
void initSystem() {
    // Populate Author Index Array
    for (int i = 0; i < total_books; i++) {
        authorIndex[i] = &bookDB[i];
    }
    // Bubble sort the Author Index array by Author name
    for (int i = 0; i < total_books - 1; i++) {
        for (int j = 0; j < total_books - i - 1; j++) {
            if (strcmp(authorIndex[j]->author, authorIndex[j+1]->author) > 0) {
                Book* temp = authorIndex[j];
                authorIndex[j] = authorIndex[j+1];
                authorIndex[j+1] = temp;
            }
        }
    }

    // Initialize Default Users
    userDB[0].id = 999; // Staff
    strcpy(userDB[0].name, "Admin Staff");
    userDB[0].is_staff = 1;
    
    userDB[1].id = 101; // Existing Member
    strcpy(userDB[1].name, "Alice (Member)");
    userDB[1].is_staff = 0;
    userDB[1].membership_start = time(NULL) - 30; // Started 30s ago
    userDB[1].membership_expiry = userDB[1].membership_start + MEMBERSHIP_DURATION;
    userDB[1].book_count = 0;
    
    total_users = 2;
}

// --- Helper Functions ---
void printTimeRemaining(double seconds) {
    if (seconds <= 0) {
        printf("EXPIRED");
        return;
    }
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    printf("%02dm:%02ds remaining", mins, secs);
}

void stripNewline(char* str) {
    str[strcspn(str, "\n")] = 0;
}

// --- Authentication & Module 2 User Creation ---
User* findUser(int id) {
    for (int i = 0; i < total_users; i++) {
        if (userDB[i].id == id) return &userDB[i];
    }
    return NULL;
}

User* createMembership(int new_id) {
    if (total_users >= MAX_USERS) {
        printf("Database full! Cannot register new members.\n");
        return NULL;
    }
    
    User* new_user = &userDB[total_users];
    new_user->id = new_id;
    new_user->is_staff = 0;
    new_user->book_count = 0;
    
    printf("\n--- Membership Registration ---\n");
    printf("Enter your Full Name: ");
    fgets(new_user->name, sizeof(new_user->name), stdin);
    stripNewline(new_user->name);
    
    new_user->membership_start = time(NULL);
    new_user->membership_expiry = new_user->membership_start + MEMBERSHIP_DURATION;
    
    total_users++;
    printf("Success! Membership generated for %s.\n", new_user->name);
    return new_user;
}

// --- Module 1: O(log n) Binary Search Algorithms ---
Book* searchByTitle(char* target) {
    int left = 0, right = total_books - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(bookDB[mid].title, target);
        if (cmp == 0) return &bookDB[mid];
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return NULL;
}

Book* searchByAuthor(char* target) {
    int left = 0, right = total_books - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(authorIndex[mid]->author, target);
        if (cmp == 0) return authorIndex[mid];
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return NULL;
}

// --- Module 1: Member Borrow Logic ---
void borrowBookProcess(User* member) {
    if (member->book_count >= MAX_BORROWED_PER_USER) {
        printf("Borrow limit reached! Please return a book first.\n");
        return;
    }

    int choice;
    char query[100];
    Book* foundBook = NULL;

    printf("\nSearch Book By:\n1. Title\n2. Author\nChoice: ");
    scanf("%d", &choice);
    getchar(); // Consume newline

    printf("Enter exact search term (Case Sensitive): ");
    fgets(query, sizeof(query), stdin);
    stripNewline(query);

    if (choice == 1) foundBook = searchByTitle(query);
    else if (choice == 2) foundBook = searchByAuthor(query);

    if (foundBook != NULL) {
        printf("\n--- Book Found ---\n");
        printf("Title: %s\nAuthor: %s\nGenre: %s\nStock: %d\n", foundBook->title, foundBook->author, foundBook->genre, foundBook->stock);
        
        if (foundBook->stock > 0) {
            // Update Book Inventory
            foundBook->stock--;
            foundBook->borrow_count++;
            
            // Update Member's Borrowed List
            BorrowedBook* new_borrow = &member->borrowed_books[member->book_count];
            new_borrow->book_id = foundBook->id;
            strcpy(new_borrow->title, foundBook->title);
            new_borrow->issue_time = time(NULL);
            new_borrow->due_time = new_borrow->issue_time + BOOK_BORROW_PERIOD;
            member->book_count++;
            
            printf("Success! You borrowed '%s'.\n", foundBook->title);
            printf("Please return it within %d seconds to avoid fines.\n", BOOK_BORROW_PERIOD);
        } else {
            printf("Sorry, this book is currently out of stock.\n");
        }
    } else {
        printf("Book not found in database.\n");
    }
}

// --- Member Dashboard (Combined M1 & M2) ---
void memberDashboard(User* member) {
    int choice;
    while (1) {
        time_t current_time = time(NULL);
        double mem_time_left = difftime(member->membership_expiry, current_time);
        
        printf("\n=========================================\n");
        printf("       MEMBER DASHBOARD: %s\n", member->name);
        printf("=========================================\n");
        
        if (mem_time_left <= 0) {
            printf("!!! YOUR MEMBERSHIP HAS EXPIRED !!!\n");
            return; // Force logout
        }
        
        printf("Membership: ACTIVE (");
        printTimeRemaining(mem_time_left);
        printf(")\n\n");
        
        // Circulation Status
        double total_fines = 0;
        printf("--- Circulation Status ---\n");
        if (member->book_count == 0) {
            printf("No books currently borrowed.\n");
        } else {
            for (int i = 0; i < member->book_count; i++) {
                printf("%d. %s\n", i+1, member->borrowed_books[i].title);
                double time_until_due = difftime(member->borrowed_books[i].due_time, current_time);
                
                if (time_until_due > 0) {
                    printf("   Due in: ");
                    printTimeRemaining(time_until_due);
                    printf("\n");
                } else {
                    double overdue = -time_until_due;
                    double fine = overdue * FINE_PER_SECOND;
                    total_fines += fine;
                    printf("   STATUS: OVERDUE by %.0f seconds! (Fine: $%.2f)\n", overdue, fine);
                }
            }
            if (total_fines > 0) printf("\n>>> TOTAL UNPAID FINES: $%.2f <<<\n", total_fines);
        }
        
        printf("\nOptions:\n1. Search & Borrow a Book\n2. Refresh Circulation Time\n3. Logout\nChoice: ");
        scanf("%d", &choice);
        getchar();
        
        if (choice == 1) borrowBookProcess(member);
        else if (choice == 3) break;
    }
}

// --- Staff Dashboard (Module 1) ---
void staffDashboard(User* staff) {
    int choice;
    
    // Auto-Alerts on Login
    printf("\n--- System Alerts ---\n");
    int alerts = 0;
    for (int i = 0; i < total_books; i++) {
        if (bookDB[i].stock <= LOW_STOCK_THRESHOLD) {
            printf("[!] ALERT: '%s' stock is critical (%d remaining)\n", bookDB[i].title, bookDB[i].stock);
            alerts++;
        }
    }
    if (alerts == 0) printf("Inventory healthy. No low stock alerts.\n");
    
    while (1) {
        printf("\n=========================================\n");
        printf("       STAFF DASHBOARD: %s\n", staff->name);
        printf("=========================================\n");
        printf("1. Update Inventory Stock\n2. Report Analysis (Most Borrowed)\n3. Logout\nChoice: ");
        scanf("%d", &choice);
        getchar();
        
        if (choice == 1) {
            char query[100];
            printf("Enter Book Title to update: ");
            fgets(query, sizeof(query), stdin);
            stripNewline(query);
            
            Book* b = searchByTitle(query);
            if (b) {
                printf("Current stock: %d. Enter new stock amount: ", b->stock);
                scanf("%d", &b->stock);
                printf("Stock updated!\n");
                if (b->stock <= LOW_STOCK_THRESHOLD) {
                    printf(">> AUTOMATIC ALERT: Stock for '%s' dropped to critical levels.\n", b->title);
                }
            } else {
                printf("Book not found.\n");
            }
        } else if (choice == 2) {
            printf("\n--- End of Cycle Report Analysis ---\n");
            int max_idx = 0;
            for (int i = 1; i < total_books; i++) {
                if (bookDB[i].borrow_count > bookDB[max_idx].borrow_count) max_idx = i;
            }
            printf("Top Borrowed Book:\n");
            printf("- Title: %s\n- Author: %s\n- Genre: %s\n- Total Circulation: %d times\n", 
                   bookDB[max_idx].title, bookDB[max_idx].author, bookDB[max_idx].genre, bookDB[max_idx].borrow_count);
        } else if (choice == 3) {
            break;
        }
    }
}

// --- Main Execution ---
int main() {
    initSystem();
    int userId;
    char response;
    
    while (1) {
        printf("\n======================================================\n");
        printf("  Unified Library Management System (Module 1 & 2)  \n");
        printf("======================================================\n");
        printf("Enter ID to Login (101: Member, 999: Staff, 0: Exit): ");
        scanf("%d", &userId);
        getchar(); 
        
        if (userId == 0) {
            printf("Shutting down system...\n");
            break;
        }
        
        User* currentUser = findUser(userId);
        
        if (currentUser != NULL) {
            if (currentUser->is_staff == 1) {
                staffDashboard(currentUser);
            } else {
                memberDashboard(currentUser);
            }
        } else {
            // Module 2 logic: Insist on membership if not found
            printf("\nID '%d' not found.\n", userId);
            printf("You must acquire a membership to access the system. Register now? (y/n): ");
            scanf("%c", &response);
            getchar();
            
            if (response == 'y' || response == 'Y') {
                currentUser = createMembership(userId);
                if (currentUser != NULL) {
                    memberDashboard(currentUser);
                }
            } else {
                printf("Access Denied: Membership required.\n");
            }
        }
    }
    return 0;
}