/* $Id: inflectedstring.h,v 1.1.2.6 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef RUSSIANSTRING_H
#define RUSSIANSTRING_H

#include "noun.h"
#include "nounholder.h"

class InflectedString : public Grammar::NounHolder,
                      public Grammar::Noun 
{
public:   
    typedef ::Pointer<InflectedString> Pointer;

    InflectedString();
    InflectedString(const DLString &ff);
    InflectedString(const DLString &ff, const Grammar::MultiGender &mg);

    virtual DLString decline(const Grammar::Case &c) const;
    virtual Grammar::Gender getGender() const;
    virtual Grammar::Number getNumber() const; 
    virtual const DLString &getFullForm() const;
    virtual NounPointer toNoun(const DLObject *forWhom = 0, int flags = 0) const;
    
    void setFullForm(const DLString &ff);
    void setGender(const Grammar::MultiGender &mg);
    const Grammar::MultiGender & getMultiGender() const { return mg; }

protected:
    void fillCachedForms();

    vector<DLString> cachedForms;
    DLString fullForm;
    Grammar::MultiGender mg;
};

#endif
