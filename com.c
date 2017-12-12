#include "com.h"

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

#include "defines.h"
#include "handlers.h"

int keepRunning;

char time_str[time_str_len];

//varibales for timing
struct timeval tv;
struct tm local_date_time;

extern char buf [wxBuffSize];
int newData;

char* getTimeArray(char ms_on_mode)
{ 
	gettimeofday(&tv, NULL);
	int msec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
	if (msec>=1000) { // Allow for rounding up to nearest second
		msec -=1000;
		tv.tv_sec++;
	}

    memcpy (&local_date_time, localtime(&tv.tv_sec), sizeof(local_date_time));
   	bzero(time_str, sizeof(time_str)); 
	if (ms_on_mode) 
		snprintf(time_str, sizeof(time_str),"%d-%02d-%02dT%02d:%02d:%02d.%03ld", 			local_date_time.tm_year+1900, local_date_time.tm_mon+1, local_date_time.tm_mday, 			local_date_time.tm_hour, local_date_time.tm_min, local_date_time.tm_sec, msec);
	else {
		snprintf(time_str, sizeof(time_str),"%d-%02d-%02d\0", 				local_date_time.tm_year+1900, local_date_time.tm_mon+1, local_date_time.tm_mday, 			local_date_time.tm_hour, local_date_time.tm_min, local_date_time.tm_sec);
	}
	return time_str;
}

int write_log(char *log_string)
{
	getTimeArray(0);
	char fname[fname_str_len];
	struct stat sb;
    if (!(stat(logPath, &sb) == 0 && S_ISDIR(sb.st_mode))) {
		char cmd_line[66];
		sprintf(cmd_line,"mkdir -m 750 %s -p", logPath);
		printf("Creating \'%s\'...", logPath);
    	if (system(cmd_line) == -1) {
			printf("failed!\n");
			perror("system");
			return -1;
		} else
			printf("OK!\n");
	}
		
	sprintf(fname,"%sMETEO_%s.log", logPath, time_str);
	FILE *file = fopen(fname, "a");
	char *copy = log_string;
	while (*copy) {
		if (*copy == '\n' && *(copy+1) == '\n') {
			*(copy+1) = '\0';
			break;
		}
		copy++;
	}
	//if (log_string[0] != '\n')
	//log_string[strcspn(log_string, "\r\n\r\n")+2] = 0;
		if (fprintf(file, "%s", log_string) < 0) {
			perror("write to log fprintf");
			fclose(file);
			return -2;
		}
   	fclose(file);
	return 0;
}

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0) {
				perror("tcgetattr");
                printf ("Error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0) {
				perror("tcsetattr");
                printf ("Error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0) {
				perror("tcgetattr");
                printf ("Error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0) {
				perror("tcgetattr");
				printf ("Error %d setting term attributes", errno);
		}

}

void *comHandler()
{		
		char port_name[15];
		stpcpy(port_name, "/dev/ttyS0");
		
		int com_fd = open (port_name, O_RDWR | O_NOCTTY | O_SYNC);
		if (com_fd < 0) {
			perror("open");
			printf ("Error %d opening %s: %s", errno, port_name, strerror (errno));
			return;
		}

		set_interface_attribs (com_fd, B9600, 0);  // set speed to 9600 bps, 8n1 (no parity)
		set_blocking (com_fd, 0);                // set no blocking

		char buf [wxBuffSize];
		memset(buf,0,wxBuffSize);
	
		int n,i;
		//int counter = 0;
		while (keepRunning) {
			n = read (com_fd, buf, wxBuffSize);	
			if (n > 0) {
				//getTimeArray(0);
				newData = n;
				//printf("\'%s\'",buf);
				strncpy(wxBuff, buf, n);
				while (newData) {
				}
				//log
				write_log(buf);
			} else if (n == -1) {
					perror("read");
					break;
			}
			memset(buf,0,wxBuffSize);
		}
		close(com_fd);
}
