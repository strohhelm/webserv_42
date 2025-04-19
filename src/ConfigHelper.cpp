#include "../include/ServerConfig.hpp"



bool	routeConfig::isDirListingActive(void)
{
    if(_dirListing)
        return true;
    else
        return false;
}