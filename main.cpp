// Student Record Manager - C++ + SQLite (SQL)
// Features: add, list, search, update marks, delete, topper/average report

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>

using namespace std;

// ---------- helpers ----------
bool runSQL(sqlite3* db, const string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        cerr << "SQL error: " << err << "\n";
        sqlite3_free(err);
        return false;
    }
    return true;
}

void printRows(sqlite3_stmt* stmt) {
    cout << left << setw(5) << "ID" << setw(22) << "Name" << setw(12)
         << "Course" << setw(8) << "Marks" << "\n"
         << string(47, '-') << "\n";
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << left << setw(5) << sqlite3_column_int(stmt, 0)
             << setw(22) << sqlite3_column_text(stmt, 1)
             << setw(12) << sqlite3_column_text(stmt, 2)
             << setw(8)  << sqlite3_column_double(stmt, 3) << "\n";
        count++;
    }
    if (count == 0) cout << "(no records)\n";
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin >> ws, s);
    return s;
}

double readNumber(const string& prompt) {
    double x;
    cout << prompt;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a valid number: ";
    }
    return x;
}

// ---------- operations (prepared statements prevent SQL injection) ----------
void addStudent(sqlite3* db) {
    string name = readLine("Name: ");
    string course = readLine("Course: ");
    double marks = readNumber("Marks (0-100): ");
    if (marks < 0 || marks > 100) { cout << "Marks must be 0-100.\n"; return; }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO students(name, course, marks) VALUES(?,?,?);", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, course.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, marks);
    if (sqlite3_step(stmt) == SQLITE_DONE) cout << "Student added.\n";
    sqlite3_finalize(stmt);
}

void listStudents(sqlite3* db) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT id, name, course, marks FROM students ORDER BY id;", -1, &stmt, nullptr);
    printRows(stmt);
    sqlite3_finalize(stmt);
}

void searchStudent(sqlite3* db) {
    string key = "%" + readLine("Search name: ") + "%";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT id, name, course, marks FROM students WHERE name LIKE ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    printRows(stmt);
    sqlite3_finalize(stmt);
}

void updateMarks(sqlite3* db) {
    int id = (int)readNumber("Student ID: ");
    double marks = readNumber("New marks (0-100): ");
    if (marks < 0 || marks > 100) { cout << "Marks must be 0-100.\n"; return; }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "UPDATE students SET marks=? WHERE id=?;", -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, marks);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    cout << (sqlite3_changes(db) ? "Updated.\n" : "ID not found.\n");
    sqlite3_finalize(stmt);
}

void deleteStudent(sqlite3* db) {
    int id = (int)readNumber("Student ID to delete: ");
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM students WHERE id=?;", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    cout << (sqlite3_changes(db) ? "Deleted.\n" : "ID not found.\n");
    sqlite3_finalize(stmt);
}

void report(sqlite3* db) {
    sqlite3_stmt* stmt;
    // Aggregate query: per-course count, average and highest marks
    sqlite3_prepare_v2(db,
        "SELECT course, COUNT(*), ROUND(AVG(marks),2), MAX(marks) "
        "FROM students GROUP BY course ORDER BY AVG(marks) DESC;", -1, &stmt, nullptr);
    cout << left << setw(12) << "Course" << setw(8) << "Count"
         << setw(10) << "Average" << setw(8) << "Top" << "\n" << string(38, '-') << "\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << left << setw(12) << sqlite3_column_text(stmt, 0)
             << setw(8)  << sqlite3_column_int(stmt, 1)
             << setw(10) << sqlite3_column_double(stmt, 2)
             << setw(8)  << sqlite3_column_double(stmt, 3) << "\n";
    }
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db,
        "SELECT name, marks FROM students ORDER BY marks DESC LIMIT 1;", -1, &stmt, nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW)
        cout << "\nTopper: " << sqlite3_column_text(stmt, 0)
             << " (" << sqlite3_column_double(stmt, 1) << ")\n";
    sqlite3_finalize(stmt);
}

// ---------- main ----------
int main() {
    sqlite3* db;
    if (sqlite3_open("students.db", &db) != SQLITE_OK) {
        cerr << "Cannot open database.\n";
        return 1;
    }
    runSQL(db,
        "CREATE TABLE IF NOT EXISTS students("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "course TEXT NOT NULL,"
        "marks REAL CHECK(marks BETWEEN 0 AND 100));");

    int choice;
    do {
        cout << "\n=== Student Record Manager ===\n"
             << "1. Add student\n2. List all\n3. Search by name\n"
             << "4. Update marks\n5. Delete student\n6. Course report\n0. Exit\n"
             << "Choice: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(10000, '\n'); choice = -1; }
        switch (choice) {
            case 1: addStudent(db);    break;
            case 2: listStudents(db);  break;
            case 3: searchStudent(db); break;
            case 4: updateMarks(db);   break;
            case 5: deleteStudent(db); break;
            case 6: report(db);        break;
            case 0: cout << "Goodbye!\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    sqlite3_close(db);
    return 0;
}
