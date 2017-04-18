/*
 * minimalistic JSON trie implementation
 * for on-the-fly and standalone JSON decoding
 * by mkdxdx
 * license: use it as you want
 */

#define NODEN(name,k,c,n)	const JSON_STRUCT name = {k,JB_NONE,c,n}
#define NODE(name,k,id,c,n)	const JSON_STRUCT name = {k,id,c,n}

enum E_JSONBLOCK_ID {
	JB_NONE = 0,
	JB_COD,				// 1
	JB_MESSAGE,			// 2
	JB_CNT,				// 3
	JB_LIST,			// 4 array block
	JB_DT,				// 5
	JB_MAIN,			// 6
	JB_TEMP,			// 7
	JB_TEMP_MIN,		// 8
	JB_TEMP_MAX,		// 9
	JB_PRESSURE,		// 10
	JB_SEA_LEVEL,		// 11
	JB_GRND_LEVEL,		// 12
	JB_HUMIDITY,		// 13
	JB_TEMP_KF,			// 14
	JB_WEATHER,			// 15 array block
	JB_ID,				// 16
	JB_DESC,			// 17
	JB_ICON,			// 18
	JB_CLOUDS,			// 19 array block
	JB_ALL,				// 20
	JB_WIND,			// 21 array block
	JB_SPEED,			// 22
	JB_DEG,				// 23
	JB_RAIN,			// 24 array block
	JB_3H,				// 25
	JB_SYS,				// 26 array block
	JB_POD,				// 27
	JB_DT_TXT,			// 28
	JB_CITY,			// 29 array block
	JB_NAME,			// 30
	JB_COORD,			// 31
	JB_LAT,				// 32
	JB_LON,				// 33
	JB_COUNTRY,			// 34
	JB_ROOT,
	_JB_LAST,
};

typedef struct {
	char key;
	enum E_JSONBLOCK_ID bnum; 	// json block id
	struct JSON_STRUCT * child, * next; // next down block (on key equal)
} JSON_STRUCT;

enum E_JSONBLOCK_ID getid(JSON_STRUCT * node, char * str);
JSON_STRUCT * getnext(JSON_STRUCT * node, char c);
enum E_JSONBLOCK_ID parseJSONString(char * str);
