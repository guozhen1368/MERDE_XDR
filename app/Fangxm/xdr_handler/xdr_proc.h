#ifndef _XDR_PROC_H_
#define _XDR_PROC_H_

#ifdef __cplusplus
extern "C"
{
#endif

int xdrproc_start(int handle_mode);

int xdrproc_add(char *xdr, int bytes);

#ifdef __cplusplus
}
#endif

#endif	/* _XDR_PROC_H_ */
