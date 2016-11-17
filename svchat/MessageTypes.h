#define LOGIN 0
#define LOGOUT 1
#define POST 2
#define MAX_MESSAGE_LEN 0x1000
typedef struct
{
	int type;
	char user_name[ 0x10 ];
	char msg[ 0 ];
} Message;
typedef struct
{
	int type;
	char user_name[ 0x10 ];
	char data[ MAX_MESSAGE_LEN ];
} MessageBlob;