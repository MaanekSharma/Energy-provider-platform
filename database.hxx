#pragma once
#include <string>
#include "sqlite3.h"

class Database {
public:
    Database(const std::string& db_name);
    ~Database();

    void createTables();
    void addCustomer(const std::string& name, const std::string& region, const std::string& email);
    void logEnergyUsage(int customer_id, const std::string& type, double kWh, const std::string& date);
    void generateBill(int customer_id, double amount_due, const std::string& due_date);
    void checkOverduePayments(const std::string& today);

private:
    sqlite3* db;
    void exec(const std::string& sql);
};
