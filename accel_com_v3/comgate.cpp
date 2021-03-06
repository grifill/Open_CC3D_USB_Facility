#include "comgate.h"

#define VERBOSE 1

#include <math.h>
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
//------------------------------------------------------------------------------

ComGate::ComGate(QObject* parent)
    : QObject(parent)
{
    fd = 0;
    memset(err_str, 0, ERR_SIZE);
    memset(&accdata, 0, sizeof(accdata));
    rxptr = 0;
    speed = 0;
    resetRotation();

    rcv = 0;
    lastRcv = 0;

    gyroCoeff = 1.56913e-05;
    gyroDrift.rx = -74;
    gyroDrift.ry = 48;
    gyroDrift.rz = -48;
    bDriftCalibration = false;

    resetRotation();
}
//------------------------------------------------------------------------------
ComGate::~ComGate()
{
    disconnectFromDevice();
}
//------------------------------------------------------------------------------
#ifdef WIN32
int ComGate::connectToDevice(const char *device, int speed, int stopbits, int parity)
#else
int ComGate::connectToDevice(const char *device, int speed, int stopbits, CG_PARITY parity)
#endif
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
        fd = 0;
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
        fd = 0;
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
    gyroTime.start();
    lastGyroTime = gyroTime.elapsed();
    resetRotation();

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
#if VERBOSE > 2
    char log[ERR_SIZE];
#endif

    if(rxptr >= RXBUF_SIZE)
    {
        rxptr = 0;  // Буфер накопленных данных сбрасывается, если за 1024
                    // байта не набралось ни одного пакета        
    }


//    while(1)
//    {
        char* arp = &(rxbuf[rxptr]);
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

#if VERBOSE > 2
        snprintf(log, ERR_SIZE, "Dump received, buf: <- %s (%d bytes)",
                 hexdump(rxbuf, rxptr),
                 rxptr);
        emit newDump(QString(log));
#endif

        // Разбираем то что прочитали
        parseData();
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
#if VERBOSE > 1
    char log[ERR_SIZE];
#endif
    for(int a=0; a<rxptr-(int)sizeof(accdata)+1; a++)
    {
        unsigned int* m = (unsigned int*)&(rxbuf[a]);
        if(*m == ACCPACK_HEADER)
        {
            // Нашли начало пакета
            // Проверим, что оно больше нигде не встречается до конца пакета
            int b=a+1;
            for(; b<a+(int)sizeof(accdata) && b<rxptr-(int)sizeof(accdata)+1; b++)
            {
                m = (unsigned int*)&(rxbuf[b]);
                if(*m == ACCPACK_HEADER)
                {
                    b = -1;
                    break;
                }
            }
            if(b<0)
                continue;   // Битый пакет

            memcpy(&accdata, rxbuf+a, sizeof(accdata));
#if VERBOSE > 1
            snprintf(log, ERR_SIZE,
                     "Packet received: %s, "
                     "Raw values: %d, %d, %d | %d, %d, %d | %g",
                     hexdump(rxbuf+a, sizeof(accdata)),
                     accdata.ax,
                     accdata.ay,
                     accdata.az,
                     accdata.gx,
                     accdata.gy,
                     accdata.gz,
                     (float)accdata.temp/256);
            emit newDump(QString(log));
#endif

            memmove(rxbuf, rxbuf+a+sizeof(accdata), rxptr-a-sizeof(accdata));
            rxptr -= a+sizeof(accdata);
            a = -1;

#if VERBOSE > 2
            snprintf(log, ERR_SIZE, "Parsed-out buf: <- %s (%d bytes), a=%d",
                     hexdump(rxbuf, rxptr),
                     rxptr,
                     a);
            emit newDump(QString(log));
#endif
        }
    }

    if(bDriftCalibration)
    {
        Rotation r;
        r.rx = accdata.gx;
        r.ry = accdata.gy;
        r.rz = accdata.gz;
        driftValues.append(r);
    }
    accdata.gx -= gyroDrift.rx;
    accdata.gy -= gyroDrift.ry;
    accdata.gz -= gyroDrift.rz;

    int dt = gyroTime.elapsed() - lastGyroTime;
    // Константы подогнаны
    rotateMatrix(gyroCoeff * (accdata.gx) * dt, -1, 0, 0);
    rotateMatrix(gyroCoeff * (accdata.gy) * dt, 0, -1, 0);
    rotateMatrix(gyroCoeff * (accdata.gz) * dt, 0, 0, 1);
    rotation.rx += gyroCoeff * (accdata.gx) * dt;
    rotation.ry += gyroCoeff * (accdata.gy) * dt;
    rotation.rz += gyroCoeff * (accdata.gz) * dt;

    lastGyroTime = gyroTime.elapsed();
}
//------------------------------------------------------------------------------
void ComGate::getData(AccPack* data, int* t)
{
    if(data)
        memcpy(data, &accdata, sizeof(accdata));
    if(t)
        memcpy(t, &lastGyroTime, sizeof(int));
}
//------------------------------------------------------------------------------
void ComGate::getRotation(Rotation *r)
{
    if(r==0)
        return;
    memcpy(r, &rotation, sizeof(rotation));
}
//------------------------------------------------------------------------------
void ComGate::getRotation(double *r)
{
    if(r==0)
        return;
    memcpy(r, &rotMatrix, 16*sizeof(double));
}
//------------------------------------------------------------------------------
void ComGate::resetRotation()
{
    memset(&rotation, 0, sizeof(rotation));
    memset(&rotMatrix, 0, 16*sizeof(double));
    rotMatrix[0] = 1;
    rotMatrix[5] = 1;
    rotMatrix[10] = 1;
    rotMatrix[15] = 1;
}
//------------------------------------------------------------------------------
void ComGate::setGyroCalibration(float k, Rotation drift)
{
    gyroCoeff = k;
    gyroDrift = drift;

    #if VERBOSE>0
    char log[ERR_SIZE];
    snprintf(log, ERR_SIZE, "Calibrated with k=%g, drift=(%g, %g, %g)",
             k,
             drift.rx,
             drift.ry,
             drift.rz);
    emit newDump(QString(log));

    #endif
}
//------------------------------------------------------------------------------
void ComGate::setGyroCalibration(float k)
{
    setGyroCalibration(k, gyroDrift);
}
//------------------------------------------------------------------------------
void ComGate::setGyroCalibration(Rotation d)
{
    setGyroCalibration(gyroCoeff, d);
}
//------------------------------------------------------------------------------
void ComGate::startDriftMeasure()
{
    driftValues.clear();
    bDriftCalibration = true;
}
//------------------------------------------------------------------------------
Rotation ComGate::getDriftMeasure()
{
    bDriftCalibration = false;
    Rotation r;
    memset(&r, 0, sizeof(r));
    for(int a=0; a<driftValues.size(); a++)
    {
        r.rx += driftValues[a].rx;
        r.ry += driftValues[a].ry;
        r.rz += driftValues[a].rz;
    }
    r.rx /= driftValues.size();
    r.ry /= driftValues.size();
    r.rz /= driftValues.size();
    driftValues.clear();
    return r;
}
//------------------------------------------------------------------------------
void ComGate::startAngleMeasure()
{
    bAngleCalibration = true;
    startAngle = rotation.rz;
}
//------------------------------------------------------------------------------
float ComGate::getAngleMeasure()
{
    bAngleCalibration = false;
    float r = fabs(rotation.rz - startAngle);
    if(r == 0)
        return 0;
    return 90.0/r*(gyroCoeff);
}
//------------------------------------------------------------------------------
void ComGate::rotateMatrix(double a, double x, double y, double z)
{
    double r[16];
    double c = cos(a*M_PI/180.0);
    double s = sin(a*M_PI/180.0);
    // Заполняем по столбцам матрицу поворота
    r[0] = x*x*(1-c) + c;
    r[1] = y*x*(1-c) + z*s;
    r[2] = x*z*(1-c) - y*s;
    r[3] = 0;

    r[4] = x*y*(1-c) - z*s;
    r[5] = y*y*(1-c) + c;
    r[6] = y*z*(1-c) + x*s;
    r[7] = 0;

    r[8] = x*z*(1-c) + y*s;
    r[9] = y*z*(1-c) - x*s;
    r[10] = z*z*(1-c) + c;
    r[11] = 0;

    r[12] = 0;
    r[13] = 0;
    r[14] = 0;
    r[15] = 1;

    double res[16];
    memset(&res, 0, 16*sizeof(double));
    // Умножение матриц
    for(int a=0; a<4; a++)
    {
        for(int b=0; b<4; b++)
        {
            for(int c=0; c<4; c++)
            {
                res[a+b*4] += rotMatrix[a+c*4] * r[b*4+c];
            }
        }
    }

    memcpy(rotMatrix, res, 16*sizeof(double));
}
