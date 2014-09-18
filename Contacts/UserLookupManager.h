// UserLookupManager.h

#pragma once

#include <map>
using namespace std;
#include "../NetworkCommon/Invitations/InvitationManager.h"

class DiplodocusContact;
class PacketDbQueryResult;

///////////////////////////////////////////////////////////////////

class UserLookupManager :  public GroupLookupInterface
{
public:
   UserLookupManager();
   ~UserLookupManager();

   void      Init();
   void      Update();
   static void Set( DiplodocusContact* contactMain ) { m_contactMain = contactMain; }

   void      SetDbIdentifier( U32 dbIdentifier ) { m_dbIdentifier = dbIdentifier; }
   U32       GetDbIdentifier() const { return m_dbIdentifier; }

   bool      HandleDbResult( PacketDbQueryResult* packet );


   bool      IsGroupValid( const string& inviteGroup ) const { return false; }
   bool      GetGroupName( const string& groupUuid, string& name ) const { return false; }
   string    GetUserName( const string& uuid ) const;
   bool      UserAddsSelfToGroup( const string& channelUuid, const string& addedUserUuid ) { return false; }
   //bool      UserAddsSelfToGroup( const string& channelUuid, const string& addedUserUuid )

protected:
   U32                           m_dbIdentifier;
   static   DiplodocusContact*   m_contactMain;

   struct UserLookupInfo // I am having scope issues and namespace conflicts.
   {
      string userName;
   };

   typedef map< stringhash, UserLookupInfo > UserMap;
   typedef pair< stringhash, UserLookupInfo > UserPair;

   UserMap                       m_userMap;
};

///////////////////////////////////////////////////////////////////