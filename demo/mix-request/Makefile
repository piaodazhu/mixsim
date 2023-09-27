all: consumer time_server
consumer: consumer.o
	$(CC) $^ -o $@
	@echo "consumer make done!"

time_server: time_server.o
	$(CC) $^ -o $@
	@echo "time_server make done!"

consumer.o:
time_server.o:

clean:
	@rm -rf *.o consumer time_server
	@echo "clean done!"

.PHONY: clean consumer time_server
