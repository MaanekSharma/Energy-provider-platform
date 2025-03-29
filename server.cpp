#include "crow_all.h"
#include <sqlite3.h>
#include <sstream>

// Helper function to retrieve customers as a JSON string
std::string getCustomersAsJson(sqlite3* db) {
    std::stringstream ss;
    ss << "[";
    sqlite3_stmt* stmt;
    const char* query = "SELECT ID, Name, Region, Balance FROM Customers;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        bool first = true;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) ss << ",";
            ss << "{";
            ss << "\"ID\": " << sqlite3_column_int(stmt, 0) << ",";
            ss << "\"Name\": \"" << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << "\",";
            ss << "\"Region\": \"" << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) << "\",";
            ss << "\"Balance\": " << sqlite3_column_double(stmt, 3);
            ss << "}";
            first = false;
        }
    }
    sqlite3_finalize(stmt);
    ss << "]";
    return ss.str();
}

int main() {
    // Initialize Crow app
    crow::SimpleApp app;

    // Open SQLite database
    sqlite3* db;
    if (sqlite3_open("energypro.db", &db)) {
        std::cerr << "Can't open database" << std::endl;
        return 1;
    }

    // Define route to get customers
    CROW_ROUTE(app, "/api/customers")
    ([db]() {
        std::string jsonStr = getCustomersAsJson(db);
        return crow::response{jsonStr};
    });

    // Run the server on port 18080
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
    return 0;
}
