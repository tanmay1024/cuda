.PHONY: 01  03 clean

GCC = gcc
NVCC = nvcc
CUDA_FLAGS = -arch=sm_86

01:
	@$(GCC) -o 01 01.c

03:
	@$(NVCC) $(CUDA_FLAGS) -o 03_cu 03.cu

clean:
	rm -f 01 02 03_cu *.o

