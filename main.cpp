#include <iostream>
#include "sqlite3.h"
#include <string>

// Callback function to print query results
static int callback(void *data, int argc, char **argv, char **azColName) {
    std::cout << "Row:\n";
    for (int i = 0; i < argc; i++) {
        std::cout << "  " << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    std::cout << std::endl;
    return 0;
}

int main() {
    sqlite3 *db;
    char *zErrMsg = nullptr;
    int rc;

    // Open (or create) the database file named "energypro.db"
    rc = sqlite3_open("energypro.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "Opened database successfully.\n";
    }

    // SQL statement to create the Customers table if it doesn't exist
    const char *sqlCreateTable =
        "CREATE TABLE IF NOT EXISTS Customers ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL, "
        "Region TEXT NOT NULL, "
        "Balance REAL"
        ");";

    rc = sqlite3_exec(db, sqlCreateTable, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (table creation): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Table created successfully.\n";
    }

    // SQL statement to insert a sample record into the Customers table
    const char *sqlInsert =
        "INSERT INTO Customers (Name, Region, Balance) "
        "VALUES ('John Doe', 'Region1', 123.45);";

    rc = sqlite3_exec(db, sqlInsert, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (insertion): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Record inserted successfully.\n";
    }

    // SQL statement to query and display all records from the Customers table
    const char *sqlSelect = "SELECT * FROM Customers;";
    std::cout << "\nQuerying the Customers table:\n";
    rc = sqlite3_exec(db, sqlSelect, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (select): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Query executed successfully.\n";
    }

    // Close the database connection
    sqlite3_close(db);
    return 0;
}
