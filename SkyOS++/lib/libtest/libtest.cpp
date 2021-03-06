// libtest.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "libdis.h"
#include "sqlite3.h"
#include <iostream>
#include <cstdlib>

using namespace std;

extern const uintptr_t heap_start;

/*void test2()
{
	sqlite3 *db;
	int rc;
	rc = sqlite3_config(SQLITE_CONFIG_MALLOC, malloc(1024 * 1024), 1024 * 1024, 32);
	fprintf(stderr, "%s:%d rc=%d\n", __FUNCTION__, __LINE__, rc);

	rc = sqlite3_open_v2("hello.db", &db, SQLITE_OPEN_READONLY, NULL);
}*/

int testSqlite()
{
	
/*
	sqlite3 *conn;
	sqlite3_stmt *statement; // SQL Statement Object
		int ret = 0;
	int cols;

	// This routine opens a connection to an SQLite database file
		//  and returns a database connection object.
		ret = sqlite3_open_v2("hello.db", &conn, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		cout << "can not open database\n";
		exit(0);
	}

	ret = sqlite3_prepare_v2(conn, "select * from hello", -1, &statement, NULL);
	if (ret != SQLITE_OK) {
		cout << "We did not get any data\n";
		exit(0);
	}

	cols = sqlite3_column_count(statement);

	for (int col = 0; col < cols; col++) {
		cout << " " << sqlite3_column_name(statement, col);
	};
	cout << endl;

	while (true) {
		ret = sqlite3_step(statement);
		if (ret == SQLITE_ROW) {
			for (int col = 0; col < cols; col++) {
				switch (sqlite3_column_type(statement, col)) {
				case SQLITE_INTEGER:
					cout << " " << sqlite3_column_int(statement, col) << " ";
					break;
				case SQLITE_FLOAT:
					cout << " " << sqlite3_column_double(statement, col) << " ";
					break;
				case SQLITE_TEXT:
					cout << " " << sqlite3_column_text(statement, col) << " ";
					break;
				case SQLITE_NULL:
					cout << " " << "NULL" << " ";
					break;
				}
			};
			cout << endl;
		}
		else if (ret == SQLITE_DONE) {
			cout << "done" << endl;
			break;
		}
		else {
			cout << "ret:" << ret << endl;
			break;
		}
	}

	sqlite3_finalize(statement);
	sqlite3_close(conn);
	*/
	return 0;
}

int main()
{
	//test2();
	x86_insn_t curr_inst;
	x86_invariant_t inv;
	char line[80];
	int pos = 0, n = 0;
	int size = 0;
	HANDLE address = GetModuleHandle(NULL);
	unsigned char* buf = (unsigned char*)address;
	x86_init(opt_none, NULL, NULL);	
	//size = x86_disasm((unsigned char*)address, 0x1000, 0, i, &curr_inst);
	int fileSize = 0x100;

	while ( pos < fileSize) {
      /* disassemble address */
      size = x86_disasm(buf, fileSize, 0, pos, &curr_inst);
      if ( size ) {
         /* print instruction */
         x86_format_insn(&curr_inst, line, 80, intel_syntax);
         printf("%s\n", line);
         pos += size;
      } else {
         printf("Invalid instruction\n");
         pos++;
      }
   }

	x86_cleanup();

    return 0;
}

