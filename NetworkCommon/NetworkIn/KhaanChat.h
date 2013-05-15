#pragma once

#include "Khaan.h"

class KhaanChat : public Khaan
{
public:
   KhaanChat( int id, bufferevent* be );
   ~KhaanChat();

   void           UpdateInwardPacketList();// this class doesn't do much with the data. It's up to the derived classes to decide what to do with it
   void           UpdateOutwardPacketList();

   int            GetConnectionId() const { return m_connectionId; }

   //void     NotifyFinishedRemoving();
   void           PreCleanup();

   void           OnLoginMessages();
   const string&  GetCurrentChannel() const { m_currentChannel; }
   void           SetCurrentChannel( const string& channel ) { m_currentChannel = channel; }
protected:
   string         m_currentChannel;
   string         m_username;
   string         m_uuid;
   int            m_connectionId;

   vector< string >  m_chatChannels;
};
