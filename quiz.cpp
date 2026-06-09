#include "quiz.h"
#include "ui_quiz.h"
#include "fftstuff.h"
#include <qtimer.h>
#include <math.h>
#include <stdlib.h>
#include <QFile>
#include <QPainter>
#include <QThread>


int test_val = 0;
int curLessonInt = 0;
int orientation [21] = {1,2,3,4,5,6,7,8,1,8,1,8,1,8,7,6,5,4,3,2,1};
QMap<QString, int> tonic_map = {
    {"G3", 43}, {"A3", 45}, {"B3", 47}, {"C4", 48}, {"D4", 50}, {"A4", 52}, {"B4", 54}, {"C5", 55}
};
QMap<int, QString> tilesGoodBadMap = {
    {0, "note0"}, {1, "note2"}, {2, "note4"}, {3, "note5"}, {4, "note7"}, {5, "note9"},
    {6, "note11"}, {7, "note12"}
};
QMap<int, int> tileKbShift = {
    {0, 0}, {1, 2}, {2, 4}, {3, 5}, {4, 7}, {5, 9}, {6, 11}, {7, 12}
};
FftStuff fts;
extern bool collectMicData;
extern double rec_arr[];    // DEFINED AS DOUBLE FOR FFTW
extern int rec_arr_cnt;
extern int frame_start;
extern int frame_size;
extern int frame_end;
extern int rec_arr_end;
QByteArray bufferReadTo;
QByteArray *currentAudioOut;
// QList<QByteArray> note_tone;
int major_number_list[] = {0,2,4,5,7,9,11,12};
int tonic_nunber;
int audio_number_list;
const QList <QString> note_letters = {"C", "C#", "D", "D#", "E",
                                     "F", "F#", "G", "G#", "A", "A#", "B" };
extern float note_acc;
extern int noteC_no;
extern int noteC_oct;
extern float nn_l[12];
extern QList<QByteArray> rawRecArrays;
int accValue = 0;
int displayDuration = 3000;
int playedNotes = 0;
int goodNotes = 0;

Microphone::Microphone(const QAudioFormat &format) : m_format(format) {
    qDebug()<<" YOU SHOULD SEE THIS ";
}

void Microphone::start()
{
    collectMicData=true;
    open(QIODevice::WriteOnly);
}

void Microphone::stop()
{
    close();
    frame_start = 0;
}

qint64 Microphone::readData(char * /* data */, qint64 /* maxlen */)      // NOT USED IN EXAMPLE
{
    return 0;
}

qreal Microphone::getNoteValue(const char *data, qint64 len) const
{
    const int channelBytes = m_format.bytesPerSample();
    const int sampleBytes = m_format.bytesPerFrame();
    const int numSamples = len / sampleBytes;

    float maxValue = 0;
    auto *ptr = reinterpret_cast<const unsigned char *>(data);

    for (int i = 0; i < numSamples; ++i) {
        float value = m_format.normalizedSampleValue(ptr);
        rec_arr[rec_arr_cnt]=value;     //  --------> DEFINED AS DOUBLE FOR FFTW  but value is a float
        //maxValue = qMax(value, maxValue);
        ptr += channelBytes;
        rec_arr_cnt++;
    };
    // qDebug()<<"    END OF DATA FROM MIC---------->   rec_arr_cnt "<<rec_arr_cnt;

    if(rec_arr_cnt > frame_end){
        fts.DoIt(frame_start, frame_size);
        frame_start = frame_end;
        frame_end = frame_end + frame_size;
    }

    if (rec_arr_cnt > 200000)
    {
        qDebug()<<"  200000 hit value " << collectMicData;
        rec_arr_cnt = 0;
        frame_start = 0;
        frame_end = frame_end + frame_size;
        qDebug() << ">>>>>>>>zero position at 200000 " << rec_arr_cnt;

        emit on_timeOut();

        qDebug() <<"                      restart here";
        qDebug() <<"                   Microphone::pos()  "<<Microphone::pos();
        test_val = 0;
    }
    test_val++;
    qDebug() <<"Microphone::pos()  "<<Microphone::pos();
    qDebug() << ">>> " << test_val;
    return maxValue;
}

qint64 Microphone::writeData(const char *data, qint64 len)
{
    if(collectMicData)
    {
        //qDebug() <<" %%%%%% :writeData(const char *data, qint64 len)  "<<&data<<" len "<<len;
        m_level = getNoteValue(data, len);
    }
    return len;
}

Speaker::Speaker()
{

}

void Speaker::start()
{
    open(QIODevice::ReadOnly);
}

void Speaker::stop()
{
    m_pos = 0;
    close();
}

void Speaker::newTest(QByteArray bufferOut)
{
    qDebug()<<"  %%%%  NEW TEST ";
    m_buffer.assign(bufferOut);
    m_buffer.clear();
    qDebug()<<&m_buffer<<" m_buffer_.size() " << m_buffer.size();
    m_buffer = bufferOut;
    qDebug()<<&m_buffer<<" m_buffer_.size() " << m_buffer.size();
}

qint64 Speaker::readData(char *data, qint64 len)
{
    qDebug() << "enter Speaker readdata...is Main? " << QThread::isMainThread();
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
            qDebug() << "chunk..." << chunk << "pos> = " << m_pos ;
        }
    }
    qDebug() << ">>is Main Thread in speaker readData: " << QThread::isMainThread();
    return total;
}

qint64 Speaker::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Speaker::bytesAvailable() const
{
    return QIODevice::bytesAvailable();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnstart_clicked()
{
    qDebug() << "starting...";
}

