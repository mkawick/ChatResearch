bin_PROGRAMS = gateway_serverd

gateway_serverddir = $(datadir)/@PACKAGE@

gateway_serverd_SOURCES = DiplodocusGateway.cpp \
KhaanConnector.cpp \
FruitadensGateway.cpp \
KhaanGateway.cpp \
Gateway.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/Utils.cpp \
../NetworkCommon/Logging/server_log.cpp

gateway_serverd_LDFLAGS = -L

gateway_serverd_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/gateway_serverd
	sudo chmod u+s $(bindir)/gateway_serverd


