

#include <stdio.h>
#include "File.h"

FileMgr::FileMgr() {
	UpdateFileList();
}

unsigned char FileMgr::QueryChunk(int3 id) {
	file_list::iterator it = fileList.find(id);

	if (it == fileList.end()) {
		char filename[32];
		print_chunk_filename(id, filename);
		FILE *fp = OpenFile(filename);
		if (fp == 0) { // no such file
			fileList.insert( std::pair<int3, unsigned char>(id, 0) );
			return 0;
		}
		else { // file found
			fileList.insert( std::pair<int3, unsigned char>(id, 1) );
			fclose(fp);
			return 1;
		}
	}
	else {
		return it->second;
	}
}

void FileMgr::UpdateFileList() {
}