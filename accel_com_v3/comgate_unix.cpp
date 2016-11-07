#include "comgate.h"





ComGate::ComGate(QObject* parent)
    : QObject(parent)
{
    fd = 0;
    memset(err_str, 0, ERR_SIZE);
    memset(bytes, 0, 4);

    connect(&speedTimer, SIGNAL(timeout()), SLOT(onSpeedTimer()));
    speedTimer.start(500);
    measure.start();

    rcv = 0;
    lastRcv = 0;
}
//------------------------------------------------------------------------------
ComGate::~ComGate()
{
    disconnectFromDevice();
}
//------------------------------------------------------------------------------
int ComGate::connectToDevice(char *device, int speed, int stopbits, int parity)
{


}
//------------------------------------------------------------------------------
void ComGate::disconnectFromDevice()
{
    close(fd);
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
char* ComGate::hexdump(UCHAR *ar, int arsize)
{
    char ret[1024];
    memset(ret, 0, 1024);
    for(int a=0; a<arsize; a++)
    {
        int ssize = strlen(ret);
        char* s = &(ret[ssize]);
        sprintf(s, "%02X ", ar[a]);
    }

    return &(ret[0]);
}
//------------------------------------------------------------------------------
#define READSIZE 100
int ComGate::readPendingData()
{

    char log[1024];

//    while(1)
//    {
        UCHAR ar[READSIZE];
        memset(ar,0,READSIZE);
        UCHAR* arp = &(ar[0]);
        // Чтение накопившихся данных
        int res = read(fd, arp, READSIZE);
        if(res < 0)    // Общая ошибка
        {
            sprintf(err_str, "Error reading from port: %d (%d bytes read)",
                    errno,
                    res );
            return CG_READ;
        }
        rcv += res;

        // Разбираем то, что прочитали
        for(int a=0; a<READSIZE-5; a++)
        {
            if((ar[a] == 0x80) && (ar[a+1] == 0x80))
            {
                bytes[0] = ar[a+2];
                bytes[1] = ar[a+3];
                bytes[2] = ar[a+4];
                break;
            }
        }

//        if(res == 0)
//            break;
        sprintf(log, "Dump received: <- %s", hexdump(arp, res));
        emit newDump(QString(log));
//    }


    return CG_OK;
}
//------------------------------------------------------------------------------
void ComGate::getAccels(char *ar)
{
    if(ar == 0)
        return;
    memcpy(ar, bytes, 3);
}
//------------------------------------------------------------------------------
void ComGate::onSpeedTimer()
{
    int el = measure.elapsed();
    speed = 1000*(rcv - lastRcv) / el;
    lastRcv = rcv;
    measure.restart();
}
