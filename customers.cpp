#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
using namespace std;

// ------------------ Class Definitions ------------------

class Customer {
public:
    int id;
    string name;
    string email;
    double balance;
    int regionId;

    Customer(int id, const string& name, const string& email, int regionId, double balance = 0.0)
        : id(id), name(name), email(email), regionId(regionId), balance(balance) {}

    void updateBalance(double amount) {
        balance += amount;
    }

    bool isOverdue() const {
        return balance > 0;
    }
};

class Region {
public:
    int id;
    string name;
    vector<shared_ptr<Customer>> customers;

    Region(int id, const string& name) : id(id), name(name) {}

    void addCustomer(shared_ptr<Customer> customer) {
        customers.push_back(customer);
    }

    double totalRevenue() const {
        double sum = 0;
        for (const auto& c : customers)
            sum += c->balance;
        return sum;
    }
};

class EnergyType {
public:
    string name;
    double rate;

    EnergyType(const string& name, double rate) : name(name), rate(rate) {}
    virtual double calculateBill(double units) const {
        return units * rate;
    }
    virtual ~EnergyType() = default;
};

class CrudeOil : public EnergyType {
public:
    CrudeOil() : EnergyType("CrudeOil", 1.5) {}
};

class Solar : public EnergyType {
public:
    Solar() : EnergyType("Solar", 2.0) {}
};

class Company {
public:
    vector<Region> regions;
    vector<shared_ptr<Customer>> allCustomers;
    vector<unique_ptr<EnergyType>> energyTypes;

    Company() {
        energyTypes.push_back(make_unique<CrudeOil>());
        energyTypes.push_back(make_unique<Solar>());
    }

    void addCustomer(shared_ptr<Customer> cust) {
        allCustomers.push_back(cust);
    }

    double totalRevenue() const {
        double sum = 0;
        for (const auto& c : allCustomers)
            sum += c->balance;
        return sum;
    }
};

// ------------------ Existing SQLite Logic ------------------

sqlite3* db = nullptr;

static int callback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\t";
    }
    std::cout << "\n";
    return 0;
}

int openDatabase(const char* filename) {
    int rc = sqlite3_open(filename, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return rc;
    }
    return rc;
}

int createCustomersTable() {
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS Customers ("
        "customer_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "email TEXT, "
        "address TEXT, "
        "region_id INTEGER, "
        "FOREIGN KEY(region_id) REFERENCES Regions(region_id)"
        ");";
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (table creation): " << zErrMsg << "\n";
        sqlite3_free(zErrMsg);
    }
    return rc;
}

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

void filterCustomersByRegion() {
    std::cout << "\nEnter Region ID (1–5): ";
    std::string input;
    std::getline(std::cin, input);
    int region_id = std::stoi(input);

    std::string sql = "SELECT * FROM Customers WHERE region_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, region_id);
    std::cout << "\n--- Customers in Region ID " << region_id << " ---\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::cout << sqlite3_column_name(stmt, i) << ": "
                      << (sqlite3_column_text(stmt, i) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)) : "NULL")
                      << "\t";
        }
        std::cout << "\n";
    }
    sqlite3_finalize(stmt);
}

void addCustomer() {
    std::cout << "\nEnter customer name: ";
    std::string name;
    std::getline(std::cin, name);

    std::cout << "Enter customer email: ";
    std::string email;
    std::getline(std::cin, email);

    std::cout << "Enter customer address: ";
    std::string address;
    std::getline(std::cin, address);

    std::cout << "Enter region ID (1–5): ";
    std::string regionInput;
    std::getline(std::cin, regionInput);
    int region_id = std::stoi(regionInput);

    std::string sql = "INSERT INTO Customers (name, email, address, region_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, region_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer added successfully.\n";
    }
    sqlite3_finalize(stmt);
}

void deleteCustomer() {
    std::cout << "\nEnter customer ID to delete: ";
    std::string idInput;
    std::getline(std::cin, idInput);
    int id = std::stoi(idInput);

    std::string sql = "DELETE FROM Customers WHERE customer_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer deleted successfully.\n";
    }
    sqlite3_finalize(stmt);
}

void updateCustomer() {
    std::cout << "\nEnter customer ID to update: ";
    std::string idInput;
    std::getline(std::cin, idInput);
    int id = std::stoi(idInput);

    std::cout << "Enter new name (blank to skip): ";
    std::string name;
    std::getline(std::cin, name);

    std::cout << "Enter new email (blank to skip): ";
    std::string email;
    std::getline(std::cin, email);

    std::cout << "Enter new address (blank to skip): ";
    std::string address;
    std::getline(std::cin, address);

    std::cout << "Enter new region ID (blank to skip): ";
    std::string region;
    std::getline(std::cin, region);

    std::string sql = "UPDATE Customers SET ";
    bool first = true;

    if (!name.empty()) {
        sql += "name = ?";
        first = false;
    }
    if (!email.empty()) {
        if (!first) sql += ", ";
        sql += "email = ?";
        first = false;
    }
    if (!address.empty()) {
        if (!first) sql += ", ";
        sql += "address = ?";
        first = false;
    }
    if (!region.empty()) {
        if (!first) sql += ", ";
        sql += "region_id = ?";
    }
    sql += " WHERE customer_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    int bindIndex = 1;
    if (!name.empty()) sqlite3_bind_text(stmt, bindIndex++, name.c_str(), -1, SQLITE_TRANSIENT);
    if (!email.empty()) sqlite3_bind_text(stmt, bindIndex++, email.c_str(), -1, SQLITE_TRANSIENT);
    if (!address.empty()) sqlite3_bind_text(stmt, bindIndex++, address.c_str(), -1, SQLITE_TRANSIENT);
    if (!region.empty()) sqlite3_bind_int(stmt, bindIndex++, std::stoi(region));
    sqlite3_bind_int(stmt, bindIndex, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Update failed: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "Customer updated successfully.\n";
    }
    sqlite3_finalize(stmt);
}

int main() {
    if (openDatabase("energypro.db") != SQLITE_OK) return 1;
    if (createCustomersTable() != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    while (true) {
        std::cout << "\n--- Customer Management Menu ---\n";
        std::cout << "1. List All Customers\n";
        std::cout << "2. Filter Customers by Region ID\n";
        std::cout << "3. Add Customer\n";
        std::cout << "4. Delete Customer\n";
        std::cout << "5. Update Customer\n";
        std::cout << "6. Exit\n";
        std::cout << "Choose option: ";

        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1") listCustomers();
        else if (choice == "2") filterCustomersByRegion();
        else if (choice == "3") addCustomer();
        else if (choice == "4") deleteCustomer();
        else if (choice == "5") updateCustomer();
        else if (choice == "6") break;
        else std::cout << "Invalid option.\n";
    }

    sqlite3_close(db);
    return 0;
}
