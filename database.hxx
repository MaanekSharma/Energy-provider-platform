#pragma once
#include <string>
#include "sqlite3.h"

class Database {
public:
    Database(const std::string& db_name);
    ~Database();

    void createTables();

    void addCustomer(const std::string& name, const std::string& email, const std::string& address, int region_id);
    void logEnergyUsage(int customer_id, int energy_type_id, const std::string& month, double units_used);
    void generateBill(int customer_id, double amount, const std::string& due_date, const std::string& status);
    void checkOverduePayments(const std::string& today);
    void exec(const std::string& sql);


private:
    sqlite3* db;
};
