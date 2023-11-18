void setup_i2c(int sda, int scl);
int setup_sensor(int id);
// reading sense_readings(MPU6050 mpu);
void get_all_readings(reading* output);
void format_readings(reading* input, char* output_buf);
void task_fifo_reset(void *flag);
