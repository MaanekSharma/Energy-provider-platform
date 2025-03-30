#include "database.hxx"

int main() {
    Database db("energypro.db");
    db.createTables();
    return 0;
}
