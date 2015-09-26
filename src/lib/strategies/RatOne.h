/**
 * @file RatOne.h
 */
#pragma once

#include "../solver/Manager.h"

namespace smtrat
{
    /**
     * Strategy description.
     *
     * @author
     * @since
     * @version
     *
     */
    class RatOne:
        public Manager
    {
        public:
            RatOne( bool _externalModuleFactoryAdding = false );
            ~RatOne();

    };

}    // namespace smtrat
