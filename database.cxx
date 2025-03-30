#include "database.hxx"
#include <iostream>

Database::Database(const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), &db))
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
}

Database::~Database() {
    sqlite3_close(db);
}

void Database::exec(const std::string& sql) {
    char* errmsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errmsg << std::endl;
        sqlite3_free(errmsg);
    }
}

void Database::createTables() {
    exec("CREATE TABLE IF NOT EXISTS Regions ("
         "region_id INTEGER PRIMARY KEY, region_name TEXT);");

    exec("CREATE TABLE IF NOT EXISTS Customers ("
         "customer_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "name TEXT, email TEXT, address TEXT, region_id INTEGER, "
         "FOREIGN KEY(region_id) REFERENCES Regions(region_id));");

    exec("CREATE TABLE IF NOT EXISTS Energy_Types ("
         "energy_type_id INTEGER PRIMARY KEY AUTOINCREMENT, type_name TEXT);");

    exec("CREATE TABLE IF NOT EXISTS Energy_Usage ("
         "usage_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "customer_id INTEGER, energy_type_id INTEGER, month TEXT, units_used REAL, "
         "FOREIGN KEY(customer_id) REFERENCES Customers(customer_id), "
         "FOREIGN KEY(energy_type_id) REFERENCES Energy_Types(energy_type_id));");

    exec("CREATE TABLE IF NOT EXISTS Bills ("
         "bill_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "customer_id INTEGER, amount REAL, due_date TEXT, paid_date TEXT, status TEXT, "
         "FOREIGN KEY(customer_id) REFERENCES Customers(customer_id));");

    exec("CREATE TABLE IF NOT EXISTS Imports_Exports ("
         "record_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "energy_type_id INTEGER, month TEXT, import_cost REAL, export_revenue REAL, "
         "FOREIGN KEY(energy_type_id) REFERENCES Energy_Types(energy_type_id));");

    exec("CREATE TABLE IF NOT EXISTS Maintenance ("
         "maintenance_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "region_id INTEGER, energy_type_id INTEGER, cost REAL, date TEXT, "
         "FOREIGN KEY(region_id) REFERENCES Regions(region_id), "
         "FOREIGN KEY(energy_type_id) REFERENCES Energy_Types(energy_type_id));");

    exec("CREATE TABLE IF NOT EXISTS At_Risk_Customers ("
         "customer_id INTEGER PRIMARY KEY, risk_amount REAL, "
         "FOREIGN KEY(customer_id) REFERENCES Customers(customer_id));");

    exec("CREATE TABLE IF NOT EXISTS Email_Logs ("
         "log_id INTEGER PRIMARY KEY AUTOINCREMENT, "
         "customer_id INTEGER, sent_date TEXT, "
         "FOREIGN KEY(customer_id) REFERENCES Customers(customer_id));");
}

void Database::addCustomer(const std::string& name, const std::string& email, const std::string& address, int region_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO Customers (name, email, address, region_id) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, address.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, region_id);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}

void Database::logEnergyUsage(int customer_id, int energy_type_id, const std::string& month, double units_used) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO Energy_Usage (customer_id, energy_type_id, month, units_used) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, customer_id);
        sqlite3_bind_int(stmt, 2, energy_type_id);
        sqlite3_bind_text(stmt, 3, month.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, units_used);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}

void Database::generateBill(int customer_id, double amount, const std::string& due_date, const std::string& status) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO Bills (customer_id, amount, due_date, status) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, customer_id);
        sqlite3_bind_double(stmt, 2, amount);
        sqlite3_bind_text(stmt, 3, due_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, status.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}

void Database::checkOverduePayments(const std::string& today) {
    std::string sql =
        "INSERT INTO At_Risk_Customers (customer_id, risk_amount) "
        "SELECT customer_id, amount FROM Bills "
        "WHERE status != 'Paid' AND julianday(?) - julianday(due_date) > 30;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, today.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}
