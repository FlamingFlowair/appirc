#ifndef ERR_CODES
#define ERR_CODES

namespace ERR {
	enum {success= 0,
			error = 255,
			eNotExist = 254,
			eMissingArg = 253,
			eNotAutorized = 252,
			eNickCollision = 251,
			eBadArg = 250
		  };
}

#endif // ERR_CODES
