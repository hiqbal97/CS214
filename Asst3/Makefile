all : client server 

client : libnetfiles.c libnetfiles.h
		gcc libnetfiles.c libnetfiles.h -o client

server : netfileserver.c
		gcc netfileserver.c -pthread -o server