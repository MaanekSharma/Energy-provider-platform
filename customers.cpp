#include <iostream>
#include <sqlite3.h>
#include <string>

// Global database handle
sqlite3* db = nullptr;

// Callback to print query results for listing customers
static int callback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\t";
    }
    std::cout << "\n";
    return 0;
}

// Open the SQLite database (or create if it doesn't exist)
int openDatabase(const char* filename) {
    int rc = sqlite3_open(filename, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return rc;
    }
    return rc;
}

// Create the Customers table if it doesn't already exist
int createCustomersTable() {
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS Customers ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL, "
        "Region TEXT NOT NULL, "
        "Balance REAL"
        ");";
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (table creation): " << zErrMsg << "\n";
        sqlite3_free(zErrMsg);
    }
    return rc;
}

// List all customers in the database
void listCustomers() {
    std::cout << "\n--- Customers List ---\n";
    std::string sql = "SELECT * FROM Customers;";
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), callback, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (list): " << zErrMsg << "\n";
        sqlite3_free(zErrMsg);
    }
}

// Add a new customer using prepared statements
void addCustomer() {
    std::cout << "\nEnter customer name: ";
    std::string name;
    std::getline(std::cin, name);

    std::cout << "Enter customer region: ";
    std::string region;
    std::getline(std::cin, region);

    std::cout << "Enter customer balance: ";
    std::string balanceInput;
    std::getline(std::cin, balanceInput);
    double balance = std::stod(balanceInput);

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO Customers (Name, Region, Balance) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, region.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, balance);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer added successfully.\n";
    }
    sqlite3_finalize(stmt);
}

// Delete a customer by ID
void deleteCustomer() {
    std::cout << "\nEnter customer ID to delete: ";
    std::string idInput;
    std::getline(std::cin, idInput);
    int id = std::stoi(idInput);

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "DELETE FROM Customers WHERE ID = ?;";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare delete statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer deleted successfully.\n";
    }
    sqlite3_finalize(stmt);
}

// Update a customer's information
void updateCustomer() {
    std::cout << "\nEnter customer ID to update: ";
    std::string idInput;
    std::getline(std::cin, idInput);
    int id = std::stoi(idInput);

    std::cout << "Enter new customer name (leave blank to keep current): ";
    std::string name;
    std::getline(std::cin, name);

    std::cout << "Enter new customer region (leave blank to keep current): ";
    std::string region;
    std::getline(std::cin, region);

    std::cout << "Enter new customer balance (leave blank to keep current): ";
    std::string balanceInput;
    std::getline(std::cin, balanceInput);

    // Dynamically build the SQL update statement
    std::string sql = "UPDATE Customers SET ";
    bool first = true;
    if (!name.empty()) {
        sql += "Name = ?";
        first = false;
    }
    if (!region.empty()) {
        if (!first) sql += ", ";
        sql += "Region = ?";
        first = false;
    }
    if (!balanceInput.empty()) {
        if (!first) sql += ", ";
        sql += "Balance = ?";
    }
    sql += " WHERE ID = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    int bindIndex = 1;
    if (!name.empty()) {
        sqlite3_bind_text(stmt, bindIndex++, name.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!region.empty()) {
        sqlite3_bind_text(stmt, bindIndex++, region.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!balanceInput.empty()) {
        double balance = std::stod(balanceInput);
        sqlite3_bind_double(stmt, bindIndex++, balance);
    }
    sqlite3_bind_int(stmt, bindIndex, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Update failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer updated successfully.\n";
    }
    sqlite3_finalize(stmt);
}

int main() {
    // Open or create the database
    if (openDatabase("energypro.db") != SQLITE_OK) {
        return 1;
    }
    
    // Create the Customers table if needed
    if (createCustomersTable() != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    // Main loop for the customer management menu
    while (true) {
        std::cout << "\n--- Customer Management ---\n";
        std::cout << "1. List Customers\n";
        std::cout << "2. Add Customer\n";
        std::cout << "3. Delete Customer\n";
        std::cout << "4. Update Customer\n";
        std::cout << "5. Exit\n";
        std::cout << "Choose an option: ";

        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1") {
            listCustomers();
        } else if (choice == "2") {
            addCustomer();
        } else if (choice == "3") {
            deleteCustomer();
        } else if (choice == "4") {
            updateCustomer();
        } else if (choice == "5") {
            break;
        } else {
            std::cout << "Invalid option, try again.\n";
        }
    }

    // Close the database before exiting
    sqlite3_close(db);
    return 0;
}
