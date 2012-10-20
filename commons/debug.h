#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG_ENABLE
  #define DEBUG(x) printf x
#else
  #define DEBUG(x) 
#endif

#endif //__DEBUG__
