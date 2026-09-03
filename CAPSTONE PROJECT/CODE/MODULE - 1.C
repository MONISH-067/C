#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BOOKS 100
#define MAX_USERS 10
#define LOW_STOCK_THRESHOLD 3

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
    int id;
    char name[50];
    int is_staff; // 0 for Member, 1 for Staff
} User;

// --- Global Databases ---
// Pre-sorted by title for O(log n) Binary Search
Book bookDB[MAX_BOOKS] = {
    {1, "A Brief History of Time", "Stephen Hawking", "Science", 2, 15},
    {2, "Clean Code", "Robert C. Martin", "Technology", 5, 42},
    {3, "Dune", "Frank Herbert", "Sci-Fi", 4, 30},
    {4, "The Great Gatsby", "F. Scott Fitzgerald", "Fiction", 1, 10},
    {5, "To Kill a Mockingbird", "Harper Lee", "Fiction", 10, 25}
};
int total_books = 5;

User userDB[MAX_USERS] = {
    {101, "Alice (Member)", 0},
    {102, "Bob (Member)", 0},
    {999, "Admin Staff", 1}
};
int total_users = 3;

// --- Authentication Module ---
User* authenticateUser(int id) {
    for (int i = 0; i < total_users; i++) {
        if (userDB[i].id == id) {
            return &userDB[i];
        }
    }
    return NULL;
}

// --- O(log n) Binary Search by Title ---
int binarySearchBook(char* targetTitle) {
    int left = 0;
    int right = total_books - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(bookDB[mid].title, targetTitle);
        
        if (cmp == 0) {
            return mid; // Book found
        }
        if (cmp < 0) {
            left = mid + 1; // Search right half
        } else {
            right = mid - 1; // Search left half
        }
    }
    return -1; // Not found
}

// --- Member Functions ---
void memberMenu(User* member) {
    int choice;
    char searchTitle[100];
    
    while (1) {
        printf("\n--- Member Menu: %s ---\n", member->name);
        printf("1. Search & Borrow Book\n");
        printf("2. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); // consume newline
        
        if (choice == 1) {
            printf("Enter exact Book Title to search: ");
            fgets(searchTitle, sizeof(searchTitle), stdin);
            searchTitle[strcspn(searchTitle, "\n")] = 0; // Remove trailing newline
            
            int index = binarySearchBook(searchTitle);
            if (index != -1) {
                printf("\nBook Found: %s by %s (Genre: %s)\n", bookDB[index].title, bookDB[index].author, bookDB[index].genre);
                if (bookDB[index].stock > 0) {
                    bookDB[index].stock--;
                    bookDB[index].borrow_count++;
                    printf("Success! You have borrowed '%s'.\n", bookDB[index].title);
                    printf("Remaining Stock: %d\n", bookDB[index].stock);
                } else {
                    printf("Sorry, '%s' is currently out of stock.\n", bookDB[index].title);
                }
            } else {
                printf("Book not found. (Note: Search is case-sensitive and requires exact match).\n");
            }
        } else if (choice == 2) {
            break;
        }
    }
}

// --- Staff Functions ---
void checkLowStock() {
    printf("\n--- Low Stock Alerts ---\n");
    int alerts = 0;
    for (int i = 0; i < total_books; i++) {
        if (bookDB[i].stock <= LOW_STOCK_THRESHOLD) {
            printf("ALERT: '%s' has low stock (%d remaining)!\n", bookDB[i].title, bookDB[i].stock);
            alerts++;
        }
    }
    if (alerts == 0) printf("All books have sufficient stock.\n");
}

void reportAnalysis() {
    printf("\n--- Report Analysis: Most Borrowed Books ---\n");
    int max_borrows = -1;
    int best_index = -1;
    
    for (int i = 0; i < total_books; i++) {
        if (bookDB[i].borrow_count > max_borrows) {
            max_borrows = bookDB[i].borrow_count;
            best_index = i;
        }
    }
    
    if (best_index != -1) {
        printf("Top Borrowed Book:\n");
        printf("Title: %s\n", bookDB[best_index].title);
        printf("Author: %s\n", bookDB[best_index].author);
        printf("Genre: %s\n", bookDB[best_index].genre);
        printf("Total Borrows: %d\n", bookDB[best_index].borrow_count);
    }
}

void staffMenu(User* staff) {
    int choice;
    char searchTitle[100];
    
    // Automatically notify low stock upon login
    checkLowStock();
    
    while (1) {
        printf("\n--- Staff Menu: %s ---\n", staff->name);
        printf("1. Update Book Stock\n");
        printf("2. View Analysis Report\n");
        printf("3. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); 
        
        if (choice == 1) {
            printf("Enter exact Book Title to update: ");
            fgets(searchTitle, sizeof(searchTitle), stdin);
            searchTitle[strcspn(searchTitle, "\n")] = 0; 
            
            int index = binarySearchBook(searchTitle);
            if (index != -1) {
                int newStock;
                printf("Current stock for '%s' is %d. Enter new stock amount: ", bookDB[index].title, bookDB[index].stock);
                scanf("%d", &newStock);
                bookDB[index].stock = newStock;
                printf("Stock updated successfully!\n");
                
                // Trigger alert if updated stock is low
                if (bookDB[index].stock <= LOW_STOCK_THRESHOLD) {
                    printf(">> AUTOMATIC ALERT: Stock for '%s' is critically low!\n", bookDB[index].title);
                }
            } else {
                printf("Book not found.\n");
            }
        } else if (choice == 2) {
            reportAnalysis();
        } else if (choice == 3) {
            break;
        }
    }
}

// --- Main Execution ---
int main() {
    int userId;
    
    printf("=========================================\n");
    printf("   Library Management System (Module 1 & 2)  \n");
    printf("=========================================\n");
    
    while (1) {
        printf("\nEnter User ID to Login (Try 101 for Member, 999 for Staff, 0 to exit): ");
        scanf("%d", &userId);
        
        if (userId == 0) {
            printf("Exiting system. Goodbye!\n");
            break;
        }
        
        User* loggedInUser = authenticateUser(userId);
        
        if (loggedInUser == NULL) {
            printf("Invalid User ID. Authentication failed.\n");
            continue;
        }
        
        // Route to appropriate dashboard based on Role
        if (loggedInUser->is_staff == 1) {
            staffMenu(loggedInUser);
        } else {
            memberMenu(loggedInUser);
        }
    }
    
    return 0;
}