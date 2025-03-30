#include "database.hxx"
#include <cstdlib>
#include <ctime>
#include <vector>

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Database db("energypro.db");

    // --- 1. Insert 5 Regions ---
    for (int i = 1; i <= 5; ++i) {
        std::string sql = "INSERT OR IGNORE INTO Regions (region_id, region_name) VALUES (" +
                          std::to_string(i) + ", 'Region_" + std::to_string(i) + "');";
        db.exec(sql);
    }

    // --- 2. Insert 500 Customers ---
    int customerId = 1;
    for (int region = 1; region <= 5; ++region) {
        for (int i = 0; i < 100; ++i) {
            db.addCustomer(
                "Customer_" + std::to_string(customerId),
                "cust" + std::to_string(customerId) + "@mail.com",
                "Address_" + std::to_string(customerId),
                region
            );
            customerId++;
        }
    }

    // --- 3. Insert Energy Types (IDs 1–4) ---
    std::vector<std::string> types = {"Crude Oil", "Solar", "Nuclear", "Natural Gas"};
    for (int i = 0; i < types.size(); ++i) {
        std::string sql = "INSERT OR IGNORE INTO Energy_Types (energy_type_id, type_name) VALUES (" +
                          std::to_string(i + 1) + ", '" + types[i] + "');";
        db.exec(sql);
    }

    // --- 4. Insert Usage Records for 6 Months ---
    const std::vector<std::string> months = {"2024-10", "2024-11", "2024-12", "2025-01", "2025-02", "2025-03"};
    for (int cust = 1; cust <= 500; ++cust) {
        for (const auto& month : months) {
            int energyTypeId = 1 + rand() % 4;
            double units = 100 + (rand() % 200); // 100–300 units
            db.logEnergyUsage(cust, energyTypeId, month, units);
        }
    }

    // --- 5. Insert Bills (some overdue) ---
    for (int cust = 1; cust <= 500; ++cust) {
        std::string due_date = (cust % 4 == 0) ? "2024-11-01" : "2025-03-15"; // ~25% overdue
        db.generateBill(cust, 100 + (rand() % 400), due_date, "Unpaid");
    }

    // --- 6. Run Overdue Checker for 2025-04-20 ---
    db.checkOverduePayments("2025-04-20");

    return 0;
}
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif

