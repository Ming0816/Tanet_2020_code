objects = main.o Hungarian.o Scheduler_Utilization_Based.o list_online.o BB_online.o Hungarian_online.o MCTS_online.o Event.o Process.o
main:$(objects)
	@g++ -std=c++11 -o main $(objects)

%.o:%.c
	@g++ -std=c++11 -c $<

.PHONY:clean
clean:
	@rm *.o
	@rm main
