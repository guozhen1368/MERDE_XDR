#ifndef _SET_KEEPALIVE_H_
#define _SET_KEEPALIVE_H_

/**
 * @brief set the keepalive option on
 * 
 * @param sock the socket will be set option
 * @param idel start keeplives after this period (seconds)
 * @param count number of keepalives before death
 * @param interval interval between keepalives (seconds)
 * 
 * @return return 0 if success or -1 if failed
 */
int set_keepalive(int sock, int idel, int count, int interval);

#endif	/* _SET_KEEPALIVE_H_ */
