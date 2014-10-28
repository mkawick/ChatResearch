
////////////////////////////////////////////////////////

template< typename PacketType, typename Processor >
bool  SendRawData( const U8* data, int size, int dataType, int maxPacketSize, U32 serverId, U8 productId, const string& identifier, U32 connectionId, U32 gatewayId, Processor* sender ) // diplodocus supposedly
{
   //cout << "Send raw data <<< " << endl;
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
      
     /* PacketGatewayWrapper* wrapper    = new PacketGatewayWrapper;
      wrapper->SetupPacket( responsePacket, connectionId );*/

      //cout << "Send raw data sending data" << endl;
      // this will be wrapped by the invoked function
      if( sender->SendPacketToGateway( responsePacket, connectionId, gatewayId ) == false )
      {
         cout << "Send raw data >>> bad send " << endl;
         cout << "   connectionId = " << connectionId << endl;
         BasePacket* tempPack = static_cast< BasePacket* >( responsePacket );
         factory.CleanupPacket( tempPack );
         
         return false;
      }
      //cout << "Send raw data send good" << endl;

      remainingSize -= sizeToSend;
      workingData += sizeToSend;// offset the pointer
      numSends --;
      if( remainingSize <= 0 )
         break;
   }
   //cout << "Send raw data >>> " << endl;
   return true;
}


////////////////////////////////////////////////////////