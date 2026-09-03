#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MEMBERS 50
#define MAX_BOOKS_PER_MEMBER 5

// Demo time constants (in seconds) so you can see real-time changes quickly
#define MEMBERSHIP_DURATION 180 
#define BOOK_BORROW_PERIOD 60   
#define FINE_PER_SECOND 2       // $2 fine for every second overdue

// --- Data Structures ---
typedef struct {
    char title[100];
    time_t issue_time;
    time_t due_time;
} BorrowedBook;

typedef struct {
    int id;
    char name[50];
    time_t membership_start;
    time_t membership_expiry;
    BorrowedBook books[MAX_BOOKS_PER_MEMBER];
    int book_count;
} Member;

// --- Global Database ---
Member memberDB[MAX_MEMBERS];
int total_members = 0;

// --- Helper Functions for Time ---
void printTimeRemaining(double seconds) {
    if (seconds <= 0) {
        printf("EXPIRED\n");
        return;
    }
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    printf("%02dm:%02ds remaining\n", mins, secs);
}

// --- Database & Member Management ---
void initializeDummyData() {
    // Create an existing member (ID: 101) with an active membership and one borrowed book
    memberDB[0].id = 101;
    strcpy(memberDB[0].name, "Alice");
    memberDB[0].membership_start = time(NULL) - 30; // Started 30 seconds ago
    memberDB[0].membership_expiry = memberDB[0].membership_start + MEMBERSHIP_DURATION;
    
    strcpy(memberDB[0].books[0].title, "Clean Code");
    memberDB[0].books[0].issue_time = time(NULL) - 50; // Borrowed 50 seconds ago
    memberDB[0].books[0].due_time = memberDB[0].books[0].issue_time + BOOK_BORROW_PERIOD;
    
    memberDB[0].book_count = 1;
    total_members = 1;
}

Member* findMember(int id) {
    for (int i = 0; i < total_members; i++) {
        if (memberDB[i].id == id) {
            return &memberDB[i];
        }
    }
    return NULL;
}

Member* createAccount(int new_id) {
    if (total_members >= MAX_MEMBERS) {
        printf("Database full! Cannot register new members.\n");
        return NULL;
    }
    
    char name[50];
    printf("\n--- Create New Membership ---\n");
    printf("Enter your Full Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline
    
    Member* new_member = &memberDB[total_members];
    new_member->id = new_id;
    strcpy(new_member->name, name);
    new_member->membership_start = time(NULL);
    new_member->membership_expiry = new_member->membership_start + MEMBERSHIP_DURATION;
    new_member->book_count = 0;
    
    total_members++;
    printf("Success! Membership created for %s.\n", new_member->name);
    return new_member;
}

// --- Dashboard Module ---
void memberDashboard(Member* member) {
    int choice;
    
    while (1) {
        time_t current_time = time(NULL);
        double mem_time_left = difftime(member->membership_expiry, current_time);
        
        printf("\n=========================================\n");
        printf("       MEMBER DASHBOARD: %s\n", member->name);
        printf("=========================================\n");
        
        if (mem_time_left <= 0) {
            printf("!!! YOUR MEMBERSHIP HAS EXPIRED !!!\n");
            printf("Please renew to continue using library services.\n");
            printf("=========================================\n");
            return; // Force logout if expired
        }
        
        printf("Membership Status: ACTIVE (");
        printTimeRemaining(mem_time_left);
        printf(")\n\n");
        
        printf("--- Borrowed Books ---\n");
        if (member->book_count == 0) {
            printf("You have no borrowed books.\n");
        } else {
            double total_fines = 0;
            
            for (int i = 0; i < member->book_count; i++) {
                printf("%d. %s\n", i+1, member->books[i].title);
                
                double time_until_due = difftime(member->books[i].due_time, current_time);
                
                if (time_until_due > 0) {
                    printf("   Time left to return: ");
                    printTimeRemaining(time_until_due);
                    printf("   Fine: $0.00\n");
                } else {
                    double overdue_seconds = -time_until_due;
                    double fine = overdue_seconds * FINE_PER_SECOND;
                    total_fines += fine;
                    printf("   STATUS: OVERDUE by %.0f seconds!\n", overdue_seconds);
                    printf("   Current Fine: $%.2f\n", fine);
                }
            }
            if (total_fines > 0) {
                printf("\n>>> TOTAL FINES ACCUMULATED: $%.2f <<<\n", total_fines);
            }
        }
        
        printf("\nOptions:\n");
        printf("1. Refresh Dashboard (Update Time & Fines)\n");
        printf("2. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();
        
        if (choice == 2) {
            printf("Logging out...\n");
            break;
        }
    }
}

// --- Main Execution ---
int main() {
    int userId;
    char response;
    
    initializeDummyData();
    
    while (1) {
        printf("\n=========================================\n");
        printf("       Library Circulation & Login       \n");
        printf("=========================================\n");
        printf("Enter your Member ID (Try 101 for demo, or a new ID to register, 0 to exit): ");
        scanf("%d", &userId);
        getchar(); 
        
        if (userId == 0) {
            printf("Exiting system...\n");
            break;
        }
        
        Member* current_member = findMember(userId);
        
        if (current_member != NULL) {
            printf("\nWelcome back, %s!\n", current_member->name);
            memberDashboard(current_member);
        } else {
            printf("\nID '%d' not found in membership database.\n", userId);
            printf("Would you like to acquire a membership? (y/n): ");
            scanf("%c", &response);
            getchar();
            
            if (response == 'y' || response == 'Y') {
                current_member = createAccount(userId);
                if (current_member != NULL) {
                    memberDashboard(current_member); // Take them straight to dashboard
                }
            } else {
                printf("Membership is required to access the library system.\n");
            }
        }
    }
    
    return 0;
}