#include "database.hxx"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <iostream>

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Database db("energypro.db");

    db.exec("DELETE FROM Customers;");
    db.exec("DELETE FROM sqlite_sequence WHERE name='Customers';");
    // Clear old bills
    db.exec("DELETE FROM Bills;");
    db.exec("DELETE FROM sqlite_sequence WHERE name='Bills';");

    std::vector<std::string> paidDates = {
        "2024-10-15", "2024-11-10", "2024-12-05",
        "2025-01-20", "2025-02-18", "2025-03-12"
    };

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

    for (int cust = 1; cust <= 500; ++cust) {
        double amount = 100 + (rand() % 400);
        std::string dueDate;
        std::string paidDate = "";
        std::string status;

        if (cust % 3 == 0) {
            // Overdue
            dueDate = "2024-12-15";
            status = "Overdue";
        } else if (cust % 3 == 1) {
            // Paid
            dueDate = "2025-03-25";
            paidDate = paidDates[rand() % paidDates.size()];
            status = "Paid";
        } else {
            // Unpaid
            dueDate = "2025-04-15";
            status = "Unpaid";
        }
        std::cout << "Customer " << cust << " → " << status << "\n";
        db.generateFullBill(cust, amount, dueDate, paidDate, status);
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
