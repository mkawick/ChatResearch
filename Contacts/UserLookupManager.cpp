#include "UserLookupManager.h"

DiplodocusContact* UserLookupManager::m_contactMain = NULL;

UserLookupManager::UserLookupManager() : GroupLookupInterface()
{
}

UserLookupManager::~UserLookupManager()
{
}


void      UserLookupManager::Init()
{
}

bool      UserLookupManager::HandleDbResult( PacketDbQueryResult* packet )
{
   return false;
}

string    UserLookupManager::GetUserName( const string& uuid ) const 
{ 
   return false; 
}