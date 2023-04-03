CC=gcc --std=c99 -g

all: keygen enc_server enc_client dec_client dec_server

keygen: keygen.c
	gcc -o keygen keygen.c

enc_server: enc_server.c
	gcc -o enc_server enc_server.c

enc_client: enc_client.c
	gcc -o enc_client enc_client.c
 
dec_server: dec_server.c
	gcc -o dec_server dec_server.c
 
dec_client: dec_client.c
	gcc -o dec_client dec_client.c