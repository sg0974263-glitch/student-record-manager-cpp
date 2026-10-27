# Student Record Manager (C++ & SQLite)

A simple console app that stores student records in an SQL database using C++.

## Features
- Add, list, search, update and delete students (CRUD)
- Course-wise report using SQL `GROUP BY`, `AVG`, `MAX`, `COUNT`
- Topper lookup using `ORDER BY ... LIMIT 1`
- Prepared statements (protects against SQL injection)
- Input validation and a `CHECK` constraint on marks (0-100)

## Tech
C++17 · SQLite3 · SQL

## Setup
Install SQLite development files:
```
# Ubuntu/Debian
sudo apt install libsqlite3-dev
# macOS
brew install sqlite
# Windows: use MSYS2 (pacman -S mingw-w64-x86_64-sqlite3) or add sqlite3.c to your project
```

## Build & Run
```
make
./student_manager
```
The database file `students.db` is created automatically on first run.

## Database schema
```sql
CREATE TABLE students(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  course TEXT NOT NULL,
  marks REAL CHECK(marks BETWEEN 0 AND 100)
);
```

## Ideas to extend
- Add a `courses` table and use a foreign key with `JOIN`
- Add login with hashed passwords
- Export records to CSV
- Build a Qt or web front-end
