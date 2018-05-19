MAKE = gmake
CC = g++49

INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj
INSTALL_DIR = /usr/game

GCC_VERSION = $(shell $(CC) --version 2>&1 | grep "(GCC)" | cut -d' ' -f3  | cut -d'.' -f1)
BSD_VERSION = $(shell uname -v 2>&1 | cut -d' ' -f2 | cut -d'.' -f1)
P4_VERSION = 55250
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

TARGET = $(BINDIR)/db

CFLAGS = -g -Wall -O2 -pipe -fno-rtti -fno-exceptions -D_THREAD_SAFE -mtune=i386 -pthread
CFLAGS += -mtune=i686 -fstack-protector-all -std=c++11

# boost
INCDIR += -I../../../Extern/include

LIBDIR += -L../../libthecore/lib -L../../libsql -L../../libpoly -L../../libgame/lib

ifeq ($(BSD_VERSION), 7)
INCDIR += -I../../libmysql/7.x-5.1.35
LIBDIR += -L../../libmysql/7.x-5.1.35
else
INCDIR += -I../../libmysql/5.x-5.1.35
LIBDIR += -L../../libmysql/5.x-5.1.35
endif

LIBS = -lthecore -lmysqlclient -lsql -lpoly -lgame -lm -lz

SRCS =	Config.cpp NetBase.cpp Peer.cpp PeerBase.cpp Main.cpp Lock.cpp DBManager.cpp \
		Cache.cpp LoginData.cpp ClientManager.cpp ClientManagerPlayer.cpp ClientManagerLogin.cpp \
		ClientManagerBoot.cpp ClientManagerParty.cpp ClientManagerGuild.cpp GuildManager.cpp HB.cpp \
		PrivManager.cpp MoneyLog.cpp ItemAwardManager.cpp ClientManagerEventFlag.cpp Marriage.cpp \
		Monarch.cpp BlockCountry.cpp ItemIDRangeManager.cpp ClientManagerHorseName.cpp version.cpp \
		ProtoReader.cpp CsvReader.cpp 

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

default: $(TARGET)

$(TARGET): $(OBJS)
	@echo ----   DB basarili bir sekilde tamamlandi. ----
	@echo ----   RAMSAY BOLTON SERVER DB: $(TARGET).... Olusturuldu
	@echo -------------------------------------------------
	@$(CC) -w $(CFLAGS) $(LIBDIR) $(OBJS) $(LIBS) -o $(TARGET)
	@touch version.cpp

$(OBJDIR)/%.o: %.cpp
	@echo Compile: $<
	@$(CC) -w $(CFLAGS) $(INCDIR) -c $< -o $@

$(OBJDIR)/version.o: version.cpp
	@$(CC) -w $(CFLAGS) -D__USER__=\"$(USER)\" -D__HOSTNAME__=\"$(HOSTNAME)\" -D__PWD__=\"$(PWD)\" -D__P4_VERSION__=\"$(P4_VERSION)\" -c $< -o $@
	@echo Compile: $<

$(OBJDIR):
	@mkdir $(OBJDIR)

clean:
	@rm -f $(OBJS) $(BINDIR)/db_r*
	
install:
	rm -rf $(INSTALL_DIR)/share/bin/db
	cp $(TARGET) $(INSTALL_DIR)/share/bin/db

dep:
	@touch Depend
	makedepend -fDepend $(INCDIR) -I/usr/include/c++/3.3 -I/usr/include/c++/4.2 -p$(OBJDIR)/ $(SRCS) 2> /dev/null

sinclude Depend
