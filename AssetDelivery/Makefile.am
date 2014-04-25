bin_PROGRAMS = asset_serverd

asset_serverddir = $(datadir)/@PACKAGE@

asset_serverd_SOURCES = AssetDelivery.cpp \
AssetOrganizer.cpp \
DiplodocusAsset.cpp \
KhaanAsset.cpp \
UserAccountAssetDelivery.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/DiplodocusServerToServer.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/CommonTypes.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/NotificationPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/Serialize.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/StatPacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/MemoryLeakTracker.cpp \
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/TableWrapper.cpp \
../NetworkCommon/Utils/Utils.cpp \
../NetworkCommon/Logging/server_log.cpp

asset_serverd_LDFLAGS = -L

asset_serverd_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/asset_serverd
	sudo chmod u+s $(bindir)/asset_serverd


