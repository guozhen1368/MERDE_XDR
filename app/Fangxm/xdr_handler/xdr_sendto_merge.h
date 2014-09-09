#ifndef _XDR_SENDTO_MERGE_H_
#define _XDR_SENDTO_MERGE_H_

#define MERGE_LEN_MIN	105	/* min length of one merge packet */
#define MERGE_LEN_MAX	326	/* max length of one merge packet */

#ifdef __cplusplus
extern "C"
{
#endif

int xdrs2m_start(int handle_mode);

int xdrs2m_send(void *val);

void xdrs2m_dump(char *buff);

#ifdef __cplusplus
}
#endif

#endif	/* _XDR_SENDTO_MERGE_H_ */
