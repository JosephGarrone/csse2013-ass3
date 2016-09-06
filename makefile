CC = gcc
CFLAGS = -g -Wall -pedantic -std=gnu99

all: started clubhub clubber finished

rebuild: clean all

clubhub: server.o errors.o structs.o utils.o cardPicker.o \
	serverConstants.h constants.h
	@echo -e "\e[4;49;37mCLUBHUB\e[0m"
	$(CC) $(CFLAGS) server.o errors.o structs.o utils.o \
		cardPicker.o -o clubhub
	@echo ""

clubber: client.o errors.o structs.o utils.o cardPicker.o \
	clientConstants.h constants.h
	@echo -e "\e[4;49;37mCLUBBER\e[0m"
	$(CC) $(CFLAGS) client.o errors.o structs.o utils.o \
		cardPicker.o -o clubber
	@echo ""

cardPicker.o: cardPicker.c cardPicker.h
	@echo -e "\e[4;49;37mCARDPICKER\e[0m"
	$(CC) $(CFLAGS) -c cardPicker.c
	@echo ""

errors.o: errors.c errors.h
	@echo -e "\e[4;49;37mERRORS\e[0m"
	$(CC) $(CFLAGS) -c errors.c
	@echo ""

server.o: server.c server.h
	@echo -e "\e[4;49;37mSERVER\e[0m"
	$(CC) $(CFLAGS) -c server.c
	@echo ""

client.o: client.c client.h
	@echo -e "\e[4;49;37mCLIENT\e[0m"
	$(CC) $(CFLAGS) -c client.c
	@echo ""

structs.o: structs.c structs.h
	@echo -e "\e[4;49;37mSTRUCTS\e[0m"
	$(CC) $(CFLAGS) -c structs.c
	@echo ""

utils.o: utils.c utils.h
	@echo -e "\e[4;49;37mUTILS\e[0m"
	$(CC) $(CFLAGS) -c utils.c
	@echo ""

clean:
	@echo -e "\e[4;49;37mCLEAN\e[0m"
	-rm *.o
	@echo -e "\e[1;49;95mClean as a whistle\e[0m\n"

finished:
	@echo -e "\e[1;49;91mBuild\e[1;49;34m finished\n\n\e[1;49;92mMay the best child segfault\e[0m\n"

started:
	@echo -e "\e[1;49;91mBuild\e[1;49;34m started\e[0m\n"
