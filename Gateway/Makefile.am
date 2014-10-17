bin_PROGRAMS = gateway_serverd

gateway_serverddir = $(datadir)/@PACKAGE@

gateway_serverd_SOURCES = ErrorCodeLookup.cpp \
FruitadensGateway.cpp \
Gateway.cpp \
KhaanGateway.cpp \
MainGatewayThread.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Logging/server_log.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanProtected.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/NetworkOut/FruitadensServerToServer.cpp \
../NetworkCommon/Packets/AnalyticsPacket.cpp \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/CommonTypes.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/InvitationPacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/NotificationPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/Serialize.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp \
../NetworkCommon/Packets/UserStatsPacket.cpp \
../NetworkCommon/Stat/StatTrackingConnections.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/MemoryLeakTracker.cpp \
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/StringUtils.cpp \
../NetworkCommon/Utils/Utils.cpp 

gateway_serverd_CXXFLAGS = -std=c++0x

gateway_serverd_LDFLAGS = -L 

gateway_serverd_LDADD = /usr/local/lib/libevent.a \
/usr/local/lib/libevent_core.a \
/usr/local/lib/libevent_pthreads.a \
/usr/local/lib/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/gateway_serverd
	sudo chmod u+s $(bindir)/gateway_serverd


