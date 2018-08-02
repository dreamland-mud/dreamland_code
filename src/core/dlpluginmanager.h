/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __DLPLUGINMANAGER_H__
#define __DLPLUGINMANAGER_H__

#include "pluginmanager.h"

class DLPluginManager : public PluginManager {
public:

    virtual DLString getTablePath( ) const;
};


#endif
