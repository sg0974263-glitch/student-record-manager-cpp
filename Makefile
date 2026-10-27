all:
	g++ -std=c++17 -Wall -o student_manager main.cpp -lsqlite3
clean:
	rm -f student_manager students.db
