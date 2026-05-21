CC      = gcc 
CFLAGS  = -Wall -Wextra -g

SRCS    = bme680/bme680.c \
          pms5003/pms5003.c \
          mpu6050/mpu6050.c \
          main_all.c

TARGET  = sentinel_test

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)