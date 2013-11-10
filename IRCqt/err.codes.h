#ifndef ERR_CODES
#define ERR_CODES

namespace ERR {
	enum {success= 0,
			error = 255,
			eNotExist = 254,
			eMissingArg = 253,
			eNotAutorized = 252,
			eNickCollision = 251,
			eBadArg = 250,
			eChannelnameCollision = 249,
			eTopicUnset = 248
		  };
}

namespace RET {
	enum { apubmsg = 128,
		   aprivmsg = 129,
		   aop = 130,
		   atopic = 131,
		   anick = 132,
		   aleave = 133,
		   akick = 134,
		   aban = 135,
		   rwall = 136,
		   ajoin = 137
		 };
}
#endif // ERR_CODES
