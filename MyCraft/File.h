
#ifndef _FILE_H_
#define _FILE_H_

#include <map>
#include "common.h"

class FileMgr {
public :
	typedef std::map<int3, unsigned char, id_compare> file_list;
	file_list fileList;

	FileMgr();
	unsigned char QueryChunk(int3 id);
	void UpdateFileList();
};

#endif