/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MOBILEPROFESSION_H__
#define __MOBILEPROFESSION_H__

#include "defaultprofession.h"

/*
 * MobileProfession
 */
class MobileProfession : public DefaultProfession
{
XML_OBJECT
public:
    typedef ::Pointer<MobileProfession> Pointer;
    
    virtual int  getThac32( Character * = NULL ) const;
    virtual int getStat( bitnumber_t, Character * = NULL ) const;
    virtual bool isPlayed( ) const;
    virtual GlobalBitvector toVector( CharacterMemoryInterface * = NULL ) const;
    virtual Flags getFlags( Character * = NULL ) const;

private:
    void checkTarget( CharacterMemoryInterface * ) const ;
};

#endif
