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

    // Route: Display admin login form (GET /admin/login)
    CROW_ROUTE(app, "/admin/login")
    ([](){
        // Return an HTML login form.
        return R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
            <meta charset="UTF-8" />
            <meta name="viewport" content="width=device-width, initial-scale=1.0" />
            <title>Admin Login - EnergyPro</title>
            <link rel="stylesheet" href="/static/style.css">
            </head>
            <body>
            <h1>Admin Login</h1>
            <form method="POST" action="/admin/login">
                <label for="username">Username:</label>
                <input type="text" id="username" name="username" required><br><br>
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required><br><br>
                <button type="submit">Login</button>
            </form>
            </body>
            </html>
        )";
    });

    // Route: Process admin login (POST /admin/login)
    CROW_ROUTE(app, "/admin/login").methods("POST"_method)
    ([](const crow::request& req){
        // Prepend "?" to ensure query_string parses it correctly.
        std::string query = "?" + req.body;
        crow::query_string qs(query.c_str());
        std::string username = qs.get("username") ? qs.get("username") : "";
        std::string password = qs.get("password") ? qs.get("password") : "";
        std::cerr << "Received username: [" << username << "], password: [" << password << "]" << std::endl;
        
        // Check against valid admin credentials.
        if ((username == "khan99@uwindsor.ca" ||
            username == "sharma9d@uwindsor.ca" ||
            username == "costasa@uwindsor.ca") &&
            password == "password123") {
            crow::response res("<p>Login successful! Go to the <a href='/admin/dashboard'>Admin Dashboard</a>.</p>");
            res.add_header("Set-Cookie", "admin_session=valid; Path=/; SameSite=Lax; Max-Age=3600");
            return res;
        } else {
            return crow::response(401, "<p>Invalid credentials. <a href='/admin/login'>Try again</a>.</p>");
        }
    });

    // Route: Admin Dashboard (GET /admin/dashboard) - Protected
    CROW_ROUTE(app, "/admin/dashboard")
    ([](const crow::request& req){
        // Check if the session cookie is set and valid.
        std::string cookie = req.get_header_value("Cookie");
        if (cookie.find("admin_session=valid") == std::string::npos) {
            return crow::response(403, "<p>Unauthorized. Please <a href='/admin/login'>login</a>.</p>");
        }
        return crow::response(
            "<h1>Admin Dashboard</h1>"
            "<p>Welcome, admin!</p>"
            "<p><a href='/admin/logout'>Logout</a></p>"
        );
    });

    // Route: Admin Logout (GET /admin/logout)
    CROW_ROUTE(app, "/admin/logout")
    ([](){
        crow::response res("<p>You have been logged out. <a href='/admin/login'>Login again</a></p>");
        res.add_header("Set-Cookie", "admin_session=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/; HttpOnly");
        return res;
    });

    // Run the server on port 18080
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
    return 0;
}
