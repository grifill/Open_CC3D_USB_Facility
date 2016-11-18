#include "comgate.h"

#define VERBOSE 0

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
/*  -----------------------------------------------------------------------  */
/*  convert integer speed to posix serial speed flags */
static int serial_speed(int speed /*  [bit/sec]  */)
{

    switch(speed) {
    case 50:
        return B50;
    case 75:
        return B75;
    case 110:
        return B110;
    case 134:
        return B134;
    case 150:
        return B150;
    case 200:
        return B200;
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 1800:
        return B1800;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default:
        return B230400;
    }

    return 0;
}
#endif

ComGate::ComGate(QObject* parent)
    : QObject(parent)
{
    fd = 0;
    memset(err_str, 0, ERR_SIZE);
    memset(&accdata, 0, sizeof(accdata));
    rxptr = 0;
    speed = 0;

    rcv = 0;
    lastRcv = 0;
}
//------------------------------------------------------------------------------
ComGate::~ComGate()
{
    disconnectFromDevice();
}
//------------------------------------------------------------------------------
int ComGate::connectToDevice(const char *device, int speed, int stopbits, CG_PARITY parity)
{
    #ifdef WIN32
    // Преобразование в вендостроку
    WCHAR dev_wchar[20];
    MultiByteToWideChar (CP_UTF8,
                         MB_PRECOMPOSED,
                         device,
                         strlen( device ),
                         dev_wchar,
                         10 );

    // Создание дескриптора
    fd = CreateFileA(//"\\\\.\\COM10",
                     device,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

    if (fd == INVALID_HANDLE_VALUE)
    {
        sprintf(err_str, "Error: CreateFileA() failed.  %ld",
                GetLastError() );
        return CG_GERR;
    }

    // Обработка события "пришли байты"
    SetCommMask(fd, EV_RXCHAR);
    // Задание размеров приемного и передающего буфера
    SetupComm(fd, 1500, 1500);

    // Пусть пока полежит так, возможно пригодится
//    COMMTIMEOUTS CommTimeOuts;
//    CommTimeOuts.ReadIntervalTimeout = 0;
//    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
//    CommTimeOuts.ReadTotalTimeoutConstant = SERIAL_TIMEOUT;
//    CommTimeOuts.WriteTotalTimeoutMultiplier = READ_TOTAL_TIMEOUT_MULTIPLIER;
//    CommTimeOuts.WriteTotalTimeoutConstant = SERIAL_TIMEOUT;

//    if (SetCommTimeouts(cont->spec.serial.fd, &CommTimeOuts) == 0)
//    {
//        sprintf(cont->err_msg,
//                "[%s:%d] Error: SetCommTimeouts() failed.  %i",
//                _FILE_, __LINE__, WSAGetLastError());
//        close(cont->spec.udp.fd);
//        return GERR_SETTIMEOUTS;
//    }

    DCB ComDCM;
    memset(&ComDCM,0,sizeof(ComDCM));
    GetCommState(fd, &ComDCM);
    ComDCM.DCBlength = sizeof(DCB);
    /* set speed */
    ComDCM.BaudRate = (DWORD)speed;
    /*  8 bits per symbol without parity  */
    ComDCM.ByteSize = 8;
    /* set parity*/
    ComDCM.Parity = parity;
    if(parity != PARITY_NONE)
        ComDCM.fParity = true;
    else
        ComDCM.fParity = false;
    /* set stop bits */
    ComDCM.StopBits = stopbits ;
    ComDCM.fAbortOnError = TRUE;
    ComDCM.fDtrControl=DTR_CONTROL_DISABLE;
    ComDCM.fRtsControl=RTS_CONTROL_DISABLE;
    ComDCM.fOutxCtsFlow=FALSE;
    ComDCM.fOutxDsrFlow=FALSE;
    ComDCM.fDsrSensitivity=FALSE;
    ComDCM.fBinary = TRUE;
    ComDCM.fInX = FALSE;
    ComDCM.fOutX = FALSE;
    ComDCM.XonChar = 0x11;
    ComDCM.XoffChar = (unsigned char)0x13;
    ComDCM.fErrorChar = FALSE;
    ComDCM.fNull = FALSE;
    ComDCM.XonLim = 128;
    ComDCM.XoffLim = 128;
    if (SetCommState(fd, &ComDCM) == 0)
    {
        sprintf(err_str, "Error: SetCommState() failed.  %ld", GetLastError());
        CloseHandle(fd);
        return CG_DCM;
    }

    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 0;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
    CommTimeOuts.ReadTotalTimeoutConstant = 20;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant = 200;

    if( SetCommTimeouts(fd, &CommTimeOuts) == 0 )
    {
        sprintf(err_str, "Error: SetCommTimeouts() failed.  %ld", GetLastError());
        CloseHandle(fd);
        return CG_TOUT;
    }
    #else
    int posix_speed=serial_speed(speed);

    fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd == -1)
    {
        fd = 0;
        sprintf(err_str, "Error: open() failed.  %i (%s)",
                errno,
                strerror(errno) );
        return CG_GERR;
    }

    struct termios options;
    memset (&options, 0, sizeof(struct termios));

    options.c_cflag = CLOCAL | CREAD;
//    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    options.c_lflag = 0;

    options.c_cc[VTIME]    = 1;   /* inter-character timer 0.1*sec */
    options.c_cc[VMIN]     = 0;   /* blocking read until n chars received */

    /*  set i/o speed  */
    cfsetispeed(&options, posix_speed);
    cfsetospeed(&options, posix_speed);

    /*  8 bits per symbol without parity  */
    options.c_cflag |= CS8;
    if(stopbits == 2)
        options.c_cflag |= CSTOPB;
    switch(parity)
    {
    case P_ODD:
        options.c_cflag |= PARODD;
    case P_EVEN:
        options.c_cflag |= PARENB;
        break;

    case P_MARK:
        options.c_cflag |= PARODD;
    case P_SPACE:
        options.c_cflag |= CMSPAR;
        options.c_cflag |= PARENB;
        break;

    case P_NONE:
    default:
        break;
    }
//    /*options.c_cflag &= ~CSIZE;*/
//    options.c_iflag &= ~ISTRIP;
//    options.c_cflag &= ~CRTSCTS;
//    options.c_iflag &= ~(IXON | IXOFF | IXANY);
//    /* no NL<->CR translation */
//    options.c_iflag &= ~( INLCR | ICRNL );
//    options.c_oflag &= ~( ONLCR | OCRNL );
//    /* raw output */
//    options.c_oflag &= ~OPOST;

    /*  set new port options  */
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        sprintf( err_str, "Error: tcsetattr() failed. %i", errno);
        close(fd);
        fd = 0;
        return CG_DCM;
    }

    #endif

    rcv = 0;
    lastRcv = 0;
    speed = 0;
    memset(&accdata, 0, sizeof(accdata));
    rxptr = 0;
    measure.start();

    sprintf(err_str, "Connected to device");
    return CG_OK;
}
//------------------------------------------------------------------------------
void ComGate::disconnectFromDevice()
{
    #ifdef WIN32
    CloseHandle(fd);
    #else
    close(fd);
    #endif
    sprintf(err_str,"Device closed.");
    fd = 0;
}
//------------------------------------------------------------------------------
char* ComGate::str_error(char *str)
{
    char *err = &(err_str[0]);
    if(str != 0)
        strcpy(str, err);

    return err;
}
//------------------------------------------------------------------------------
char ret[1024];
char* ComGate::hexdump(const char *ar, int arsize)
{
    memset(ret, 0, 1024);
    for(int a=0; a<arsize; a++)
    {
        int ssize = strlen(ret);
        char* s = &(ret[ssize]);
        sprintf(s, "%02X ", ar[a]&0xFF);
    }

    return &(ret[0]);
}
//------------------------------------------------------------------------------
int ComGate::readPendingData()
{
#if VERBOSE > 1
    char log[ERR_SIZE];
#endif

    if(rxptr >= RXBUF_SIZE)
    {
        rxptr = 0;  // Буфер накопленных данных сбрасывается, если за 1024
                    // байта не набралось ни одного пакета
    }


//    while(1)
//    {
        char* arp = &(rxbuf[0]);
        // Чтение накопившихся данных
        #ifdef WIN32
        int readsize = RXBUF_SIZE-rxptr;
        int res = 0;
        int bOp = ReadFile(fd, arp, readsize, (DWORD*)&res, 0);
        if(bOp == 0)    // Общая ошибка
        {
            snprintf(err_str, ERR_SIZE, "Error reading from port: %ld (%d bytes read)",
                    GetLastError(),
                    res );
            return CG_READ;
        }
        #else
        int res = read(fd, arp, RXBUF_SIZE-rxptr);
        if(res < 0)    // Общая ошибка
        {
            sprintf(err_str, "Error reading from port: %d (%d bytes read)",
                    errno,
                    res );
            return CG_READ;
        }
        #endif
//        if(res == 0)
//            break;

        rcv += res;
        rxptr += res;

        // Разбираем то что прочитали
        parseData();
#if VERBOSE > 1
        snprintf(log, ERR_SIZE, "Dump received: <- %s", hexdump(arp, res));
        emit newDump(QString(log));
#endif
//    }
    if(measure.elapsed() > 500)
    {
        speed = 1000*(rcv - lastRcv) / measure.elapsed();
        lastRcv = rcv;
        measure.restart();
    }
    return CG_OK;
}
//------------------------------------------------------------------------------
void ComGate::parseData()
{
#if VERBOSE > 0
    char log[ERR_SIZE];
#endif
    for(int a=0; a<rxptr-4-(int)sizeof(accdata); a++)
    {
        unsigned int* m = (unsigned int*)&(rxbuf[a]);
        if(*m == 0xDEADBEEF)
        {
            // Нашли начало пакета
#if VERBOSE > 0
            snprintf(log, ERR_SIZE,
                     "Packet received: %s",
                     hexdump(rxbuf+a, 4+sizeof(accdata)));
            emit newDump(QString(log));
#endif
            memcpy(&accdata, rxbuf+a+4, sizeof(accdata));
            memmove(rxbuf, rxbuf+a+4+sizeof(accdata), rxptr-a-4-sizeof(accdata));
            rxptr -= a+4+sizeof(accdata);
            a = -1;

        }
    }
}
//------------------------------------------------------------------------------
void ComGate::getData(AccPack* data)
{
    if(data == 0)
        return;
    memcpy(data, &accdata, sizeof(accdata));
}
