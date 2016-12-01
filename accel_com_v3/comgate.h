#ifndef COMGATE_H
#define COMGATE_H

#ifdef WIN32
#include <windows.h>
#else
#define UCHAR unsigned char
// Тип четности
enum CG_PARITY
{
    P_NONE,     // Отутствует
    P_EVEN,     // Дополнение до четности
    P_ODD,      // Дополнение до нечетности
    P_MARK,     // Бит четности всегда 1
    P_SPACE     // Бит четности всегда 0
};
#endif
#include <stdio.h>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QList>
#include <stdint.h>

#define ERR_SIZE    1024        // Размер строки с описанием ошибки
#define RXBUF_SIZE  1024        // Размер буфера данных

#define CG_OK       0       // Успешный код возврата
#define CG_GERR     -1      // Общая ошибка
#define CG_DCM      -2      // Ошибка настройки порта
#define CG_TOUT     -3      // Ошибка установки таймаута
#define CG_READ     -4      // Ошибка чтения
#define GC_OVERRUN  -5      // Переполнение буфера чтения


#pragma pack (push,1)

// Пакет с данными устройства.
// в LittleEndian
#define ACCPACK_HEADER 0xDEADBEEF
struct AccPack
{
    uint32_t header;    // Маркер пакета
    int16_t ax;     // Координаты вектора ускорения
    int16_t ay;
    int16_t az;
    int16_t gx;     // Координаты положения гироскопа
    int16_t gy;
    int16_t gz;
    int16_t temp;   // Температура
};
#pragma pack (pop)

// Структура с информацией о углах поворота (интеграл от показаний гироскопа)
// в градусах
struct Rotation
{
    float rx;
    float ry;
    float rz;
};

class ComGate: public QObject
{
    Q_OBJECT

public:
    ComGate(QObject* parent=0);
    ~ComGate();

    #ifdef WIN32
    // Подключение к устройству
    int connectToDevice(const char* device = "COM1",
                        int speed = 115200,
                        int stopbits = ONESTOPBIT,
                        int parity = PARITY_NONE );
    #else
    int connectToDevice(const char* device = "/dev/ttyACM0",
                        int speed = 115200,
                        int stopbits = 1,
                        CG_PARITY parity = P_NONE );
    #endif
    // Отключение от устройства
    void disconnectFromDevice();

    bool isConnected() { return (fd != 0); }

    // Возвращает строковое описание ошибки. Если str != 0, записывает его
    // также и туда
    char* str_error(char* str = 0);

    // Возвращает строковое описание ошибки диспенсера (ASCII)
    char* err_desc(UCHAR err_code);

    // Записывает ускорения в переданную структуру data и время приема данных
    // гироскопа
    void getData(AccPack* data, int* t = 0);

    // Записывает вращение в переденную структуру
    void getRotation(Rotation* r);

    // Сбрасывает накопленные углы поворота
    void resetRotation();

    // Заносит константы калибровки гироскопа
    void setGyroCalibration(float k, Rotation d);
    void setGyroCalibration(float k);
    void setGyroCalibration(Rotation d);

    // Запуск измерения нуля гироскопа
    void startDriftMeasure();

    // Остановка измерения нуля и получение усредненных результатов
    Rotation getDriftMeasure();

    // Запуск измерения к-та поворота гироскопа
    void startAngleMeasure();

    // Возвращает коэффициент поворота гироскопа, считая, что устройство
    // повернули на 90 градусов вокруг вертикальной оси
    float getAngleMeasure();

    // Чтение данных в последовательном порту с минимальным таймаутом
    int readPendingData();

    // Возвращает скорость чтения порта в Б/с
    double getSpeed() { return speed; }

private:
    #ifdef WIN32
    HANDLE fd;
    #else
    int fd;
    #endif
    char err_str[ERR_SIZE];     // Строка с сообщением об ошибке
    char rxbuf[RXBUF_SIZE];     // Прочитанные байты
    int rxptr;                  // Сколько байт прочитано в rxbuf
    AccPack accdata;            // Последние прочитанные данные устройства
    QTime gyroTime;             // Считалка времени измерений гироскопа
    int lastGyroTime;           // Время последнего снятия показаний гироскопа
    Rotation rotation;          // Углы поворота, проинтегрированные из
                                // показаний гироскопа
    float gyroCoeff;            // Коэффициент перед интегралом скорости
    Rotation gyroDrift;         // Уход нуля гироскопа

    int rcv, lastRcv;           // Для подсчета скорости
    double speed;               // Скорость, байт/с
    QTime measure;              // Время последнего замера скорости

    bool bDriftCalibration;     // Проводится ли калибровка нуля
    QList<Rotation> driftValues;// Массив для измерения дрейфа гироскопа

    bool bAngleCalibration;     // Проводится ли калибровка к-та расчета угла
    float startAngle;           // Начальный угол

    // Возвращает строку, соответствующую хексдампу переданного массива
    char* hexdump(const char *ar, int arsize);

    // Разбирает данные в rxbuf, раскладывает все по переменным класса и
    // сдвигает массив.
    void parseData();

signals:
    void newDump(QString dump);
};

#endif // COMGATE_H
