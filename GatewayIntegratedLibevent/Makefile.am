bin_PROGRAMS = gateway_serverd

gateway_serverddir = $(datadir)/@PACKAGE@

gateway_serverd_SOURCES = DiplodocusGateway.cpp \
FruitadensGateway.cpp \
Gateway.cpp \
KhaanGateway.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/Serialize.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/NetworkOut/FruitadensServerToServer.cpp \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DataBucket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/StatPacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp \
../NetworkCommon/Stat/StatTrackingConnections.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/MemoryLeakTracker.cpp \
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/Utils.cpp \
../NetworkCommon/Logging/server_log.cpp

gateway_serverd_CXXFLAGS = -std=c++0x

gateway_serverd_LDFLAGS = -L 

gateway_serverd_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/gateway_serverd
	sudo chmod u+s $(bindir)/gateway_serverd


