#include "crow_all.h"
#include <sqlite3.h>
#include <sstream>
#include <string>

// CORS Middleware
struct SimpleCORS {
    struct context {};
    void before_handle(crow::request& req, crow::response& res, context&) {
        if (req.method == "OPTIONS"_method) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.code = 204;
            res.end();
        }
    }
    void after_handle(crow::request&, crow::response& res, context&) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }
};

int createRegionsTable(sqlite3* db) {
    std::string sql =
        "CREATE TABLE IF NOT EXISTS Regions ("
        "region_id INTEGER PRIMARY KEY, "
        "region_name TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (Regions): " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
    return rc;
}

int createCustomersTable(sqlite3* db) {
    std::string sql =
        "CREATE TABLE IF NOT EXISTS Customers ("
        "customer_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "email TEXT, "
        "address TEXT, "
        "region_id INTEGER, "
        "FOREIGN KEY(region_id) REFERENCES Regions(region_id)"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (Customers): " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
    return rc;
}

std::string getCustomersAsJson(sqlite3* db, const std::string& filterQuery, const std::string& paramValue = "") {
    std::stringstream ss;
    ss << "[";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, filterQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (!paramValue.empty())
            sqlite3_bind_text(stmt, 1, paramValue.c_str(), -1, SQLITE_TRANSIENT);

        bool first = true;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) ss << ",";
            ss << "{";
            ss << "\"customer_id\": " << sqlite3_column_int(stmt, 0) << ",";
            ss << "\"name\": \"" << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << "\",";
            ss << "\"email\": \"" << (sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "") << "\",";
            ss << "\"address\": \"" << (sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "") << "\",";
            ss << "\"region_id\": " << sqlite3_column_int(stmt, 4);
            ss << "}";
            first = false;
        }
    }
    sqlite3_finalize(stmt);
    ss << "]";
    return ss.str();
}

std::string getRegionsAsJson(sqlite3* db) {
    std::stringstream ss;
    ss << "[";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT * FROM Regions;", -1, &stmt, nullptr) == SQLITE_OK) {
        bool first = true;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) ss << ",";
            ss << "{";
            ss << "\"region_id\": " << sqlite3_column_int(stmt, 0) << ",";
            ss << "\"region_name\": \"" << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << "\"";
            ss << "}";
            first = false;
        }
    }
    sqlite3_finalize(stmt);
    ss << "]";
    return ss.str();
}

int main() {
    crow::App<SimpleCORS> app;

    sqlite3* db;
    if (sqlite3_open("energypro.db", &db)) {
        std::cerr << "Can't open database" << std::endl;
        return 1;
    }

    if (createRegionsTable(db) != SQLITE_OK || createCustomersTable(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    CROW_ROUTE(app, "/api/customers")
    ([db](const crow::request& req) {
        std::string sql;
        std::string param;
        auto action = req.url_params.get("action");

        if (!action || std::string(action) == "manage") {
            sql = "SELECT customer_id, name, email, address, region_id FROM Customers;";
        } else if (std::string(action) == "filter") {
            if (auto region = req.url_params.get("region")) {
                sql = "SELECT customer_id, name, email, address, region_id FROM Customers WHERE region_id = ?;";
                param = region;
            }
        } else if (std::string(action) == "lookupByName") {
            if (auto name = req.url_params.get("name")) {
                sql = "SELECT customer_id, name, email, address, region_id FROM Customers WHERE name LIKE '%' || ? || '%';";
                param = name;
            }
        } else if (std::string(action) == "edit") {
            if (auto id = req.url_params.get("id")) {
                sql = "SELECT customer_id, name, email, address, region_id FROM Customers WHERE customer_id = ?;";
                param = id;
            }
        } else if (std::string(action) == "delete") {
            crow::json::wvalue res;
            if (auto id = req.url_params.get("id")) {
                sql = "DELETE FROM Customers WHERE customer_id = ?;";
                sqlite3_stmt* stmt = nullptr;
                if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_TRANSIENT);
                    if (sqlite3_step(stmt) == SQLITE_DONE) {
                        res["message"] = "Customer deleted successfully.";
                    } else {
                        res["error"] = sqlite3_errmsg(db);
                    }
                } else {
                    res["error"] = sqlite3_errmsg(db);
                }
                sqlite3_finalize(stmt);
            } else {
                res["error"] = "No ID provided.";
            }
            crow::response r((res.count("error") > 0) ? 500 : 200);
            r.write(res.dump());
            r.set_header("Content-Type", "application/json");
            return r;
        }

        std::string jsonStr = getCustomersAsJson(db, sql, param);
        crow::response r{200};
        r.write(jsonStr);
        r.set_header("Content-Type", "application/json");
        return r;
    });

    // POST: Add new customer
    CROW_ROUTE(app, "/api/customers").methods("POST"_method)
    ([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        crow::json::wvalue res;
        if (!body || !body.has("Name") || !body.has("Region_ID")) {
            res["error"] = "Missing required fields.";
            crow::response r(400);
            r.write(res.dump());
            r.set_header("Content-Type", "application/json");
            return r;
        }

        std::string name = body["Name"].s();
        std::string email = body.has("Email") ? std::string(body["Email"].s()) : std::string("");
        std::string address = body.has("Address") ? std::string(body["Address"].s()) : std::string("");

        int region_id = body["Region_ID"].i();

        std::string sql = "INSERT INTO Customers (name, email, address, region_id) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, address.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, region_id);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                res["message"] = "Customer added successfully.";
            } else {
                res["error"] = sqlite3_errmsg(db);
            }
            sqlite3_finalize(stmt);
        } else {
            res["error"] = sqlite3_errmsg(db);
        }

        crow::response r((res.count("error") > 0) ? 500 : 200);
        r.write(res.dump());
        r.set_header("Content-Type", "application/json");
        return r;
    });

    CROW_ROUTE(app, "/api/bills")
([db](const crow::request& req) {
    std::string statusFilter;
    if (auto status = req.url_params.get("status")) {
        statusFilter = std::string(status);
    }

    std::string sql = "SELECT bill_id, customer_id, amount, due_date, paid_date, status FROM Bills";
    if (!statusFilter.empty()) {
        sql += " WHERE status = ?";
    }

    sqlite3_stmt* stmt = nullptr;
    crow::json::wvalue result;
    std::vector<crow::json::wvalue> billList;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (!statusFilter.empty()) {
            sqlite3_bind_text(stmt, 1, statusFilter.c_str(), -1, SQLITE_TRANSIENT);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue row;
            row["bill_id"] = sqlite3_column_int(stmt, 0);
            row["customer_id"] = sqlite3_column_int(stmt, 1);
            row["amount"] = sqlite3_column_double(stmt, 2);
            row["due_date"] = (const char*)sqlite3_column_text(stmt, 3);
            row["paid_date"] = sqlite3_column_text(stmt, 4) ? (const char*)sqlite3_column_text(stmt, 4) : "";
            row["status"] = (const char*)sqlite3_column_text(stmt, 5);
            billList.push_back(row);
        }
    }

    sqlite3_finalize(stmt);
    result["bills"] = std::move(billList);

    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.write(result.dump());
    return res;
});



    // GET: List regions
    CROW_ROUTE(app, "/api/regions")
    ([db]() {
        std::string jsonStr = getRegionsAsJson(db);
        crow::response r{200};
        r.write(jsonStr);
        r.set_header("Content-Type", "application/json");
        return r;
    });

    CROW_ROUTE(app, "/api/emailReminder")
    ([db](const crow::request& req) {
        auto idParam = req.url_params.get("customer_id");
        crow::json::wvalue res;

        if (!idParam) {
            res["error"] = "No customer_id provided";
            return crow::response(400, res);
        }

        int id = std::stoi(idParam);
        std::string sql = "INSERT INTO Email_Logs (customer_id, sent_date) VALUES (?, date('now'));";
        sqlite3_stmt* stmt = nullptr;

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

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                res["message"] = "Reminder email logged.";
            } else {
                res["error"] = sqlite3_errmsg(db);
            }
        } else {
            res["error"] = sqlite3_errmsg(db);
        }
        sqlite3_finalize(stmt);
        return crow::response(200, res);
    });

    
    app.port(18080).multithreaded().run();
    sqlite3_close(db);
    return 0;
}
