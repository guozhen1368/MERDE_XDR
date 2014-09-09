#ifndef _SET_COREFILE_H_
#define _SET_COREFILE_H_

#define R_UNLIMITED	(~0UL)

/**
 * @brief set core file to specific size
 * 
 * @param size the core file's size
 * 
 * @return return zero if success return zero, or return -1 if error
 */
int set_corefile(const unsigned long long size);

#endif	/* _SET_COREFILE_H_ */
