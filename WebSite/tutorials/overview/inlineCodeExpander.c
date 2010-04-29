//---------------------------------------------------------
// Example of an 'inline code expander' as proposed by
// Jack D. Herrington.
//
// Type:
//   CodeWorker -translate inlineCodeExpander.cwp
//       inlineCodeExpander.sqlc inlineCodeExpander.c
//---------------------------------------------------------

#include "db.h" 

int main( int argc, char *argv[] ) 
{
	// '< SQL select * from users >' is replaced by C code
	DBHandle* db_handle = db_connect();
	DBQueryData* db_data = db_query(db_handle, "select * from users");
	for (int record = 0; record < db_data->length; record++) {
		// fetch and process data
	}
	return 0;
}