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
    exec("CREATE TABLE IF NOT EXISTS Customers ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "name TEXT, region TEXT, email TEXT);");

    exec("CREATE TABLE IF NOT EXISTS EnergyUsage ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "customer_id INTEGER, type TEXT, kWh REAL, date TEXT);");

    exec("CREATE TABLE IF NOT EXISTS Billing ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "customer_id INTEGER, amount_due REAL, due_date TEXT, paid INTEGER DEFAULT 0);");

    exec("CREATE TABLE IF NOT EXISTS Risk ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "customer_id INTEGER, overdue_amount REAL, notified INTEGER DEFAULT 0);");
}

void Database::addCustomer(const std::string& name, const std::string& region, const std::string& email) {
    std::string sql = "INSERT INTO Customers (name, region, email) VALUES ('" +
                      name + "', '" + region + "', '" + email + "');";
    exec(sql);
}

void Database::logEnergyUsage(int customer_id, const std::string& type, double kWh, const std::string& date) {
    std::string sql = "INSERT INTO EnergyUsage (customer_id, type, kWh, date) VALUES (" +
                      std::to_string(customer_id) + ", '" + type + "', " + std::to_string(kWh) + ", '" + date + "');";
    exec(sql);
}

void Database::generateBill(int customer_id, double amount_due, const std::string& due_date) {
    std::string sql = "INSERT INTO Billing (customer_id, amount_due, due_date) VALUES (" +
                      std::to_string(customer_id) + ", " + std::to_string(amount_due) + ", '" + due_date + "');";
    exec(sql);
}

void Database::checkOverduePayments(const std::string& today) {
    std::string sql =
        "INSERT INTO Risk (customer_id, overdue_amount) "
        "SELECT customer_id, amount_due FROM Billing "
        "WHERE paid = 0 AND julianday('" + today + "') - julianday(due_date) > 30;";
    exec(sql);
}
