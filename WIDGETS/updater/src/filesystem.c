
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include "filesystem.h"
#include <malloc.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>

int filesystem_loadfiletoram(const char * path, void ** data_ptr, size_t * data_sz_ptr) {
	HANDLE hFile;
	DWORD bytesRead;
	LARGE_INTEGER liFileSize;
	char * buf = NULL;
	int res = -1;
	BOOL brc;
	hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}
	brc = GetFileSizeEx(hFile, &liFileSize);
	if (brc == 0) {
		goto error;
	}
	// We are going to null terminate it!
	buf = malloc((size_t)liFileSize.QuadPart + 1);
	if (buf == NULL) {
		goto error;
	}
	buf[(size_t)liFileSize.QuadPart] = '\0';
	brc = ReadFile(hFile, buf, (DWORD)(liFileSize.QuadPart), &bytesRead, NULL);
	if (brc == FALSE) {
		goto error;
	}
	*data_ptr = buf;
	*data_sz_ptr = (size_t)liFileSize.QuadPart;
	CloseHandle(hFile);
	return 0;
error:
	if (buf) {
		free(buf);
	}
	CloseHandle(hFile);
	return res;
}

int filesystem_saveramtofile(const char * path, const void * data, const size_t size) {
	HANDLE hFile;
	DWORD bytesWritten;
	int res = -1;
	BOOL brc;
	hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return res;
	}
	/*
	hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	*/
	brc = WriteFile(hFile, data, (DWORD)size, &bytesWritten, NULL);
	SetEndOfFile(hFile);
	if (brc == FALSE || bytesWritten != size) {
		goto error;
	}
	res = 0;
error:
	CloseHandle(hFile);
	return res;
}

int filesystem_fileexists(const char * path) {
	HANDLE hfile;
	hfile = CreateFileA((LPCSTR)path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return 0;
	}
	CloseHandle(hfile);
	return 1;
}

int filesystem_deletefile(const char * path) {
	int res = -1;
	if (DeleteFileA(path)) {
		res = 0;
	} else {
		if (filesystem_fileexists(path)) {
			res = -2;
		} else {
			res = 0;
		}
	}
	return res;
}

int filesystem_movefile(const char * dst_filename, const char * src_filename) {
	BOOL bool_rc;
	bool_rc = MoveFileExA(src_filename, dst_filename, MOVEFILE_REPLACE_EXISTING);
	if (!bool_rc) {
		return -1;
	}
	return 0;
}

/*
 * StackOverflow:
 * https://stackoverflow.com/questions/2038912/how-to-recursively-traverse-directories-in-c-on-windows
 */

static wchar_t * uihelpers_utf8towidechar(const char * input) {
	int input_len;
	input_len = (int)strlen(input);
	wchar_t * result;
	int result_len;
	/* the result length does not include the terminating null character */
	result_len = MultiByteToWideChar(CP_UTF8, 0, input, input_len, NULL, 0);
	if (result_len == 0) {
		return NULL;
	}
	result = (wchar_t *)malloc((result_len + 1) * sizeof(wchar_t));
	if (result == NULL) {
		return NULL;
	}
	MultiByteToWideChar(CP_UTF8, 0, input, input_len, result, result_len);
	result[result_len] = '\0';
	return result;
}

static char * uihelpers_widechartoutf8(const wchar_t * input) {
	int input_len;
	input_len = (int)wcslen(input);
	char * result;
	int result_len;
	/* the result length does not include the terminating null character */
	result_len = WideCharToMultiByte(CP_UTF8, 0, input, input_len, NULL, 0, NULL, NULL);
	if (result_len == 0) {
		return NULL;
	}
	result = (char *)malloc(result_len + 1);
	if (result == NULL) {
		return NULL;
	}
	WideCharToMultiByte(CP_UTF8, 0, input, input_len, result, result_len, NULL, NULL);
	result[result_len] = '\0';
	return result;
}

// PathCombine is part of shlwapi
static int FindFilesRecursively(void * userdata, int (*cb)(void * userdata, const char * path), LPCTSTR lpFolder, LPCTSTR lpFilePattern, int depth) {
	TCHAR szFullPattern[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	int rc = 0;
	// first we are going to process any subdirectories
	PathCombine(szFullPattern, lpFolder, _T("*"));
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if(hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if ((_tcscmp(_T("."), FindFileData.cFileName) == 0) || (_tcscmp(_T(".."), FindFileData.cFileName) == 0)) {
				} else {
					// found a subdirectory; recurse into it
					if (depth) {
						PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
						rc = FindFilesRecursively(userdata, cb, szFullPattern, lpFilePattern, depth - 1);
						if (rc != 0) {
							goto first_finished;
						}
					}
				}
			}
		} while(FindNextFile(hFindFile, &FindFileData));
first_finished:
		FindClose(hFindFile);
	}

	// Now we are going to look for the matching files
	PathCombine(szFullPattern, lpFolder, lpFilePattern);
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if(hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				// found a file; do something with it
				PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
				//_tprintf_s(_T("%s\n"), szFullPattern);
				char * full_path_and_filename = uihelpers_widechartoutf8(szFullPattern);
				rc = cb(userdata, full_path_and_filename);
				free(full_path_and_filename);
				if (rc != 0) {
					goto second_finished;
				}
			}
		} while(FindNextFile(hFindFile, &FindFileData));
second_finished:
		FindClose(hFindFile);
	}
	return rc;
}

int filesystem_recursedirectories(void * userdata, filesystem_callback cb, const char * path, int depth) {
	wchar_t * wpath;
	int rc;
	wpath = uihelpers_utf8towidechar(path);
	rc = FindFilesRecursively(userdata, cb, wpath, L"*", depth);
	free(wpath);
	return rc;
}

#endif /* _WIN32 */

#ifndef _WIN32

#define _POSIX_C_SOURCE 200809L
/* include definition for DT_DIR */
#define _DEFAULT_SOURCE

#include "filesystem.h"
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#ifdef FILESYSTEM_FEATURE_ISOLDERTHAN
#include "timefn.h"

int filesystem_isfileolderthan(char * path, int seconds) {
	struct stat fstat;
	int rc;
	rc = lstat(path, &fstat);
	if (rc == -1) {
		return -1;
	}
	if (fstat.st_mtime < (timefn_getcurrentunixtime() - seconds)) {
		return 1;
	}
	return 0;
}
#endif

int filesystem_saveramtofile(const char * path, const void * data, const size_t size) {
	int rc, fd;
	ssize_t wrsz;
	fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) {
		return -1;
	}
	wrsz = write(fd, data, size);
	rc = ftruncate(fd, size);
	close(fd);
	if (wrsz != size) {
		unlink(path);
		return -1;
	}
	if (rc) {
		return -1;
	}
	return 0;
}

int filesystem_loadfiletoram(const char * path, void ** data_ptr, size_t * data_sz_ptr) {
	int rc, fd = -1;
	void * data;
	ssize_t data_sz;
	struct stat file_stat;
	rc = lstat(path, &file_stat);
	if (rc == -1 || S_ISDIR(file_stat.st_mode)) {
		return -1;
	}
	data = malloc(file_stat.st_size + 1);
	if (data == NULL) {
		return -1;
	}
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		goto error;
	}
	data_sz = read(fd, data, file_stat.st_size);
	if (data_sz == -1) {
		goto error;
	}
	*(((char *)data) + data_sz) = '\0';
	close(fd);
	*data_ptr = data;
	*data_sz_ptr = (size_t)data_sz;
	return 0;
error:
	if (data) {
		free(data);
	}
	if (fd != -1) {
		close(fd);
	}
	return -1;
}

int filesystem_fileexists(const char * path) {
	int fd;
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	close(fd);
	return 1;
}

int filesystem_deletefile(const char * path) {
	int rc;
	rc = unlink(path);
	if (rc) {
		if (!filesystem_fileexists(path)) {
			return 0;
		}
		return -1;
	}
	return 0;
}

int filesystem_movefile(const char * dst_filename, const char * src_filename) {
	int rc;
	rc = unlink(dst_filename);
	if (rc) {
		if (errno != ENOENT) {
			return -1;
		}
	}
	rc = link(src_filename, dst_filename);
	if (rc == 0) {
		rc = unlink(src_filename);
	}
	if (rc) {
		return -1;
	}
	return 0;
}

char * filesystem_sanitisedir(char * path) {
	size_t path_len;
	char * sane_path;
	path_len = strlen(path);
	if (path_len == 0) {
		return strdup(path);
	}
	if (path[path_len - 1] == '/') {
		return strdup(path);
	}
	sane_path = malloc(path_len + 2);
	if (sane_path == NULL) {
		return NULL;
	}
	memcpy(sane_path, path, path_len);
	sane_path[path_len] = '/';
	path_len ++;
	sane_path[path_len] = '\0';
	return sane_path;
}

/*
 * path is mutated and should be at least FILESYSTEM_PATH_MAXLEN
 */
static int filesystem_recursedirectories_internal(void * user_data, filesystem_callback fscb, char * path, int depth) {
	DIR * dir;
	struct dirent * entry;
	struct stat status;
	int rc;
	int res = -1;
	size_t path_len;
	char * path_end;

	path_len = strlen(path);
	path_end = path + (path_len - 1);
	if (*path_end == '/') {
		*path_end = '\0';
	}
	if (!(dir = opendir(path))) {
		return res;
	}
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			size_t path_len;
			char * path_pos;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			path_len = strlen(path);
			path_pos = path + path_len;
			*path_pos = '/';
			strcpy(path_pos + 1, entry->d_name);
			rc = lstat(path, &status);
			if (rc == -1) {
				continue;
			}
			// not following directory symbolic links for now.
			if (S_ISLNK(status.st_mode)) {
				continue;
			}
			rc = 0;
			if (depth) {
				rc = filesystem_recursedirectories_internal(user_data, fscb, path, depth - 1);
			}
			*path_pos = '\0';
			if (rc) {
				res = rc;
				goto fs_rd_end;
			}
		} else {
			path_len = strlen(path);
			size_t filename_len = strlen(entry->d_name);
			char * end_of_path;
			end_of_path = path + path_len;
			*end_of_path = '/';
			if ((path_len + 1 + filename_len + 1) >= FILESYSTEM_PATH_MAXLEN) {
				res = -1;
				goto fs_rd_end;
			}
			strcpy(end_of_path + 1, entry->d_name);
			//fprintf(stdout, "%s\n", path);
			rc = fscb(user_data, path);
			*end_of_path = '\0';
			if (rc) {
				res = rc;
				goto fs_rd_end;
			}
		}
	}
	res = 0;
fs_rd_end:
	closedir(dir);
	return res;
}

int filesystem_recursedirectories(void * user_data, filesystem_callback fscb, const char * path, int depth) {
	char * mutated_path;
	int res;
	mutated_path = malloc(FILESYSTEM_PATH_MAXLEN);
	if (!mutated_path) {
		return -1;
	}
	strcpy(mutated_path, path);
	res = filesystem_recursedirectories_internal(user_data, fscb, mutated_path, depth);
	free(mutated_path);
	return res;
}

#endif /* ! _WIN32 */
