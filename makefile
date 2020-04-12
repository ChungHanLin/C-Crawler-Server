all:
	gcc -lcurl main.c socket.c md5.c log.c hash/db/ip_map.c hash/db/url_db.c hash/db/seen_db.c hash/db/fail_db.c hash/dns/cache.c hash/hash.c
clean:
	rm a.out
