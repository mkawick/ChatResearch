
////////////////////////////////////////////////////////

template< typename PacketType, typename Processor >
bool  SendRawData( const U8* data, int size, int dataType, int maxPacketSize, U32 serverId, U8 productId, const string& identifier, U32 connectionId, Processor* sender ) // diplodocus supposedly
{
   PacketFactory factory;
   const U8* workingData = data;
   int remainingSize = size;
   int numSends = remainingSize / maxPacketSize + 1;

   while( numSends > 0 )
   {
      int sizeToSend = remainingSize;
      if( sizeToSend > maxPacketSize )
      {
         sizeToSend = maxPacketSize;
      }

      //cout << "index:" << numSends << ", bytes:" << sizeToSend << endl;

      PacketType* responsePacket = new PacketType();
            
      responsePacket->Prep( sizeToSend, workingData, numSends );
      responsePacket->identifier       = identifier;
      responsePacket->gameInstanceId   = serverId;
      responsePacket->gameProductId    = productId;
      responsePacket->dataType         = dataType;
      
      PacketGatewayWrapper* wrapper    = new PacketGatewayWrapper;
      wrapper->SetupPacket( responsePacket, connectionId );

      if( sender->AddOutputChainData( wrapper, connectionId ) == false )
      {
         BasePacket* tempPack = static_cast< BasePacket* >( wrapper );
         factory.CleanupPacket( tempPack );
         
         return false;
      }

      remainingSize -= sizeToSend;
      workingData += sizeToSend;// offset the pointer
      numSends --;
      if( remainingSize <= 0 )
         break;
   }
   return true;
}


////////////////////////////////////////////////////////