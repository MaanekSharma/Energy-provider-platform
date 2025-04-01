#include "database.hxx"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Database db("energypro.db");

    std::vector<std::string> months = {"2024-10", "2024-11", "2024-12", "2025-01", "2025-02", "2025-03"};

    for (int energyTypeId = 1; energyTypeId <= 4; ++energyTypeId) {
        for (const auto& month : months) {
            double importCost = 1000 + rand() % 5000;
            double exportRevenue = 500 + rand() % 3000;

            std::string sql =
                "INSERT INTO Imports_Exports (energy_type_id, month, import_cost, export_revenue) "
                "VALUES (" + std::to_string(energyTypeId) + ", '" + month + "', " +
                std::to_string(importCost) + ", " + std::to_string(exportRevenue) + ");";

            db.exec(sql);
        }
    }

    return 0;
}
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif

