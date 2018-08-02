/* $Id: algo.h,v 1.1.4.1.6.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __ALGO_H__
#define __ALGO_H__

// MOC_SKIP_BEGIN

template <typename F, typename T>
struct Conv {
    typedef char (&Small);
    typedef char (&Big)[2];

    static Small cast(...);
    static Big cast(T *);
    static F *makeF();
    
    enum { exists = (sizeof(cast(makeF())) == sizeof(Big)) };
};

template <bool B, class V1, class V2> struct Choice;
template <class V1, class V2> struct Choice<true, V1, V2> : public V1 { };
template <class V1, class V2> struct Choice<false, V1, V2> : public V2 { };

// MOC_SKIP_END

#endif

